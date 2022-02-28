#include <core.h>
#include <str.h>
#include <log.h>
#include <platform.h>
#include "luascript.h"
#include "luabot.h"
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
	// {"bot", luaopen_bot},
	{"client", luaopen_client},
	{"config", luaopen_config},
	{"command", luaopen_command},
	{"vector", luaopen_vector},
	{"angle", luaopen_angle},
	{"color", luaopen_color},
	{"survival", luaopen_survival},

	{NULL,NULL}
};

LuaScript *lua_getscript(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "__plstruct");
	LuaScript *ud = (LuaScript *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return ud;
}

cs_bool LuaScript_DoMainFile(LuaScript *script) {
	if(script->unloaded) return false;
	cs_char path[MAX_PATH] = {0};
	String_Append(path, MAX_PATH, "scripts/");
	String_Append(path, MAX_PATH, script->scrname);
	if(luaL_dofile(script->L, path)) {
		LuaScript_PrintError(script);
		return false;
	}

	if(LuaScript_GlobalLookup(script, "onStart"))
		return LuaScript_Call(script, 0, 0);

	return true;
}

cs_bool LuaScript_GlobalLookup(LuaScript *script, cs_str key) {
	if(script->unloaded) return false;
	lua_getglobal(script->L, key);
	if(lua_isnil(script->L, -1)) {
		lua_pop(script->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaScript_RegistryLookup(LuaScript *script, cs_str regtab, cs_str key) {
	if(script->unloaded) return false;
	lua_getfield(script->L, LUA_REGISTRYINDEX, regtab);
	lua_getfield(script->L, -1, key);
	if(lua_isnil(script->L, -1)) {
		lua_pop(script->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaScript_Call(LuaScript *script, int args, int ret) {
	if(script->unloaded) return false;

	if(lua_pcall(script->L, args, ret, 0) != 0) {
		LuaScript_PrintError(script);
		return false;
	}

	return true;
}

static int allowhotreload(lua_State *L) {
	luaL_checktype(L, 1, LUA_TBOOLEAN);
	LuaScript *script = lua_getscript(L);
	script->hotreload = (cs_bool)lua_toboolean(L, 1);
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

LuaScript *LuaScript_Open(cs_str name) {
	LuaScript *script = Memory_TryAlloc(1, sizeof(LuaScript));

	if(script) {
		script->scrname = String_AllocCopy(name);
		script->lock = Mutex_Create();
		script->L = luaL_newstate();
		if(!script->L) {
			LuaScript_Close(script);
			return NULL;
		}

		lua_pushlightuserdata(script->L, script);
		lua_setfield(script->L, LUA_REGISTRYINDEX, "__plstruct");

		lua_pushcfunction(script->L, allowhotreload);
		lua_setglobal(script->L, "allowHotReload");

		lua_pushcfunction(script->L, sleepmillis);
		lua_setglobal(script->L, "sleepMillis");

		for(const luaL_Reg *lib = lualibs; lib->func; lib++) {
#			if LUA_VERSION_NUM < 502
				lua_pushcfunction(script->L, lib->func);
				lua_pushstring(script->L, lib->name);
				lua_call(script->L, 1, 0);
#			else
				luaL_requiref(script->L, lib->name, lib->func, 1);
				lua_pop(script->L, 1);
#			endif
		}

		if(LuaScript_GlobalLookup(script, LUA_IOLIBNAME)) {
			for(cs_int32 i = 0; iodel[i]; i++) {
				lua_pushnil(script->L);
				lua_setfield(script->L, -2, iodel[i]);
			}
			lua_pushcfunction(script->L, dir_ensure);
			lua_setfield(script->L, -2, "ensure");
			lua_pop(script->L, 1);
		}

		if(LuaScript_GlobalLookup(script, LUA_OSLIBNAME)) {
			for(cs_int32 i = 0; osdel[i]; i++) {
				lua_pushnil(script->L);
				lua_setfield(script->L, -2, osdel[i]);
			}
			lua_pop(script->L, 1);
		}

		if(LuaScript_GlobalLookup(script, "log")) {
			lua_getfield(script->L, -1, "info");
			if(lua_isfunction(script->L, -1)) {
				lua_setglobal(script->L, "print");
				lua_pop(script->L, 1);
			} else
				lua_pop(script->L, 2);
		}

		if(!LuaScript_DoMainFile(script)) {
			LuaScript_Close(script);
			return NULL;
		}

		return script;
	}

	return NULL;
}

cs_bool LuaScript_Close(LuaScript *script) {
	if(script->L) lua_close(script->L);
	Memory_Free((void *)script->scrname);
	Mutex_Free(script->lock);
	Memory_Free(script);
	return false;
}
