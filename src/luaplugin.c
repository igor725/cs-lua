#include <core.h>
#include <str.h>
#include <log.h>
#include <platform.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luaworld.h"
#include "luacommand.h"
#include "lualog.h"

static const luaL_Reg lualibs[]={
	{"", luaopen_base},
	{"package", luaopen_package},
	{"table", luaopen_table},
	{"math", luaopen_math},
	{"string", luaopen_string},
	{"client", luaopen_client},
	{"world", luaopen_world},
	{"vector", luaopen_vector},
	{"angle", luaopen_angle},
	{"command", luaopen_command},
	{"debug", luaopen_debug},
	// {"log", luaopen_log},
	{NULL,NULL}
};

LuaPlugin *lua_getplugin(lua_State *L) {
	lua_pushstring(L, "__plstruct");
	lua_gettable(L, LUA_REGISTRYINDEX);
	LuaPlugin *ud = (LuaPlugin *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return ud;
}

static cs_bool DoFile(LuaPlugin *plugin) {
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
	lua_getglobal(plugin->L, key);
	if(lua_isnil(plugin->L, -1)) {
		lua_pop(plugin->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaPlugin_RegistryLookup(LuaPlugin *plugin, cs_str regtab, cs_str key) {
	lua_pushstring(plugin->L, regtab);
	lua_gettable(plugin->L, LUA_REGISTRYINDEX);
	lua_pushstring(plugin->L, key);
	lua_gettable(plugin->L, -2);
	if(lua_isnil(plugin->L, -1)) {
		lua_pop(plugin->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaPlugin_Call(LuaPlugin *plugin, int args, int ret) {
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

		lua_pushstring(plugin->L, "__plstruct");
		lua_pushlightuserdata(plugin->L, plugin);
		lua_settable(plugin->L, LUA_REGISTRYINDEX);

		lua_pushcfunction(plugin->L, allowhotreload);
		lua_setglobal(plugin->L, "allowHotReload");

		for(const luaL_Reg *lib = lualibs; lib->func; lib++){
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
