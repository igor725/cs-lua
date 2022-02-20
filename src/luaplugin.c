#include <core.h>
#include <str.h>
#include <log.h>
#include <platform.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luacolor.h"
#include "luaworld.h"
#include "luacommand.h"
#include "lualog.h"
#include "luasurvival.h"

static const luaL_Reg lualibs[] = {
	{"", luaopen_base},
	{"package", luaopen_package},
	{"debug", luaopen_debug},
	{"table", luaopen_table},
	{"math", luaopen_math},
	{"string", luaopen_string},
	{"client", luaopen_client},
	{"world", luaopen_world},
	{"vector", luaopen_vector},
	{"angle", luaopen_angle},
	{"color", luaopen_color},
	{"command", luaopen_command},
#ifdef LUA_BITLIBNAME
	{LUA_BITLIBNAME, luaopen_bit},
#endif
#ifdef LUA_JITLIBNAME
	{LUA_FFILIBNAME, luaopen_ffi},
	{LUA_JITLIBNAME, luaopen_jit},
#endif
	// {"log", luaopen_log},
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

		lua_pushcfunction(plugin->L, luasurv_request);
		lua_setglobal(plugin->L, "requestSurvivalInterface");

		for(const luaL_Reg *lib = lualibs; lib->func; lib++) {
			lua_pushcfunction(plugin->L, lib->func);
			lua_pushstring(plugin->L, lib->name);
			lua_call(plugin->L, 1, 0);
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
