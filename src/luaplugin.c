#include <core.h>
#include <str.h>
#include <log.h>
#include <platform.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luacolor.h"
#include "luablock.h"
#include "luaworld.h"
#include "luacommand.h"
#include "lualog.h"
#include "luasurvival.h"
#include "luaconfig.h"

static const luaL_Reg lualibs[] = {
	{"", luaopen_base},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_TABLIBNAME, luaopen_table},
	{LUA_IOLIBNAME, luaopen_io},
	{LUA_OSLIBNAME, luaopen_os},
	{LUA_LOADLIBNAME, luaopen_package},
	{LUA_DBLIBNAME, luaopen_debug},
#ifdef LUA_BITLIBNAME
	{LUA_BITLIBNAME, luaopen_bit},
#endif
#ifdef LUA_JITLIBNAME
	{LUA_FFILIBNAME, luaopen_ffi},
	{LUA_JITLIBNAME, luaopen_jit},
#endif

	{"log", luaopen_log},
	{"block", luaopen_block},
	{"world", luaopen_world},
	{"client", luaopen_client},
	{"config", luaopen_config},
	{"command", luaopen_command},
	{"vector", luaopen_vector},
	{"angle", luaopen_angle},
	{"color", luaopen_color},
	{"survival", luaopen_survival},

	{NULL,NULL}
};

LuaPlugin *lua_getplugin(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "__plstruct");
	LuaPlugin *ud = (LuaPlugin *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return ud;
}

static cs_bool DoFile(LuaPlugin *plugin) {
	if(plugin->unloaded) return false;
	cs_char path[MAX_PATH] = {0};
	String_Append(path, MAX_PATH, "scripts/");
	String_Append(path, MAX_PATH, plugin->scrname);
	if(luaL_dofile(plugin->L, path)) {
		LuaPlugin_PrintError(plugin);
		return false;
	}

	if(LuaPlugin_GlobalLookup(plugin, "onStart"))
		return LuaPlugin_Call(plugin, 0, 0);

	return true;
}

cs_bool LuaPlugin_GlobalLookup(LuaPlugin *plugin, cs_str key) {
	if(plugin->unloaded) return false;
	lua_getglobal(plugin->L, key);
	if(lua_isnil(plugin->L, -1)) {
		lua_pop(plugin->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaPlugin_RegistryLookup(LuaPlugin *plugin, cs_str regtab, cs_str key) {
	if(plugin->unloaded) return false;
	lua_getfield(plugin->L, LUA_REGISTRYINDEX, regtab);
	lua_getfield(plugin->L, -1, key);
	if(lua_isnil(plugin->L, -1)) {
		lua_pop(plugin->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaPlugin_Call(LuaPlugin *plugin, int args, int ret) {
	if(plugin->unloaded) return false;

	if(lua_pcall(plugin->L, args, ret, 0) != 0) {
		LuaPlugin_PrintError(plugin);
		return false;
	}

	return true;
}

static int allowhotreload(lua_State *L) {
	luaL_checktype(L, 1, LUA_TBOOLEAN);
	LuaPlugin *plugin = lua_getplugin(L);
	plugin->hotreload = (cs_bool)lua_toboolean(L, 1);
	return 0;
}

static int sleepmillis(lua_State *L) {
	lua_Integer ms = luaL_checkinteger(L, 1);
	luaL_argcheck(L, ms > 0, 1, "Sleep timeout must be greater than zero");
	Thread_Sleep((cs_uint32)ms);
	return 0;
}

static cs_str iodel[] = {
	"input", "output", "stdout",
	"stderr", "stdin", "read",
	"write", NULL
};

static cs_str osdel[] = {
	"setlocale", "execute", "exit",
	"getenv", NULL
};

static int dir_ensure(lua_State *L) {
	cs_str path = luaL_checkstring(L, 1);
	lua_pushboolean(L, Directory_Ensure(path));
	return 1;
}

LuaPlugin *LuaPlugin_Open(cs_str name) {
	LuaPlugin *plugin = Memory_TryAlloc(1, sizeof(LuaPlugin));

	if(plugin) {
		plugin->scrname = String_AllocCopy(name);
		plugin->lock = Mutex_Create();
		plugin->L = luaL_newstate();
		if(!plugin->L) {
			LuaPlugin_Close(plugin);
			return NULL;
		}

		lua_pushlightuserdata(plugin->L, plugin);
		lua_setfield(plugin->L, LUA_REGISTRYINDEX, "__plstruct");

		lua_pushcfunction(plugin->L, allowhotreload);
		lua_setglobal(plugin->L, "allowHotReload");

		lua_pushcfunction(plugin->L, sleepmillis);
		lua_setglobal(plugin->L, "sleepMillis");

		for(const luaL_Reg *lib = lualibs; lib->func; lib++) {
#if LUA_VERSION_NUM < 502
			lua_pushcfunction(plugin->L, lib->func);
			lua_pushstring(plugin->L, lib->name);
			lua_call(plugin->L, 1, 0);
#else
			luaL_requiref(plugin->L, lib->name, lib->func, 1);
			lua_pop(plugin->L, 1);
#endif
		}

		if(LuaPlugin_GlobalLookup(plugin, LUA_IOLIBNAME)) {
			for(cs_int32 i = 0; iodel[i]; i++) {
				lua_pushnil(plugin->L);
				lua_setfield(plugin->L, -2, iodel[i]);
			}
			lua_pushcfunction(plugin->L, dir_ensure);
			lua_setfield(plugin->L, -2, "ensure");
			lua_pop(plugin->L, 1);
		}

		if(LuaPlugin_GlobalLookup(plugin, LUA_OSLIBNAME)) {
			for(cs_int32 i = 0; osdel[i]; i++) {
				lua_pushnil(plugin->L);
				lua_setfield(plugin->L, -2, osdel[i]);
			}
			lua_pop(plugin->L, 1);
		}

		if(LuaPlugin_GlobalLookup(plugin, "log")) {
			lua_getfield(plugin->L, -1, "info");
			if(lua_isfunction(plugin->L, -1)) {
				lua_setglobal(plugin->L, "print");
				lua_pop(plugin->L, 1);
			} else
				lua_pop(plugin->L, 2);
		}

		if(!DoFile(plugin)) {
			LuaPlugin_Close(plugin);
			return NULL;
		}

		return plugin;
	}

	return NULL;
}

cs_bool LuaPlugin_Reload(LuaPlugin *plugin) {
	LuaPlugin_Lock(plugin);
	if(plugin->unloaded) return false;

	if(LuaPlugin_GlobalLookup(plugin, "preReload")) {
		if(!LuaPlugin_Call(plugin, 0, 1)) {
			noreload:
			LuaPlugin_Unlock(plugin);
			return false;
		}
		if(!lua_isnil(plugin->L, -1) && !lua_toboolean(plugin->L, -1)) {
			lua_pop(plugin->L, 1);
			goto noreload;
		}
	}

	DoFile(plugin);
	LuaPlugin_Unlock(plugin);

	return true;
}

cs_bool LuaPlugin_Close(LuaPlugin *plugin) {
	if(plugin->unloaded) return false;
	if(plugin->L) lua_close(plugin->L);
	Memory_Free((void *)plugin->scrname);
	Mutex_Free(plugin->lock);
	Memory_Free(plugin);
	return false;
}
