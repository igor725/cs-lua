#include <core.h>
#include <str.h>
#include <log.h>
#include <platform.h>
#include "luascript.h"
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
#include "luacuboid.h"
#include "luagroups.h"
#include "luamodel.h"
#include "luakeys.h"

// Слой совместимости для чистой версии Lua 5.1
#ifdef CSLUA_NONJIT_51
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
	for(; l->name != NULL; l++) {
		int i;
		for(i = 0; i < nup; i++)
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, l->func, nup);
		lua_setfield(L, -(nup + 2), l->name);
	}
	lua_pop(L, nup);
}

void luaL_setmetatable(lua_State *L, const char *tname) {
	luaL_getmetatable(L, tname);
	lua_setmetatable(L, -2);
}

void *luaL_testudata (lua_State *L, int ud, const char *tname) {
	void *p = lua_touserdata(L, ud);
	if(p != NULL) {
		if(lua_getmetatable(L, ud)) {
			luaL_getmetatable(L, tname);
			if(!lua_rawequal(L, -1, -2))
				p = NULL;
			lua_pop(L, 2);
			return p;
		}
	}
	return NULL;
}
#endif

int lua_checktabfield(lua_State *L, int idx, cs_str fname, int ftype) {
	lua_getfield(L, idx, fname);
	if(lua_type(L, -1) != ftype) {
		luaL_error(L, "Field '%s' must be a %s", fname, lua_typename(L, ftype));
		return false;
	}

	return true;
}

int lua_checktabfieldud(lua_State *L, int idx, cs_str fname, const char *meta) {
	lua_getfield(L, idx, fname);
	if(!luaL_testudata(L, -1, meta)) {
		luaL_error(L, "Field '%s' must be a %s", fname, meta);
		return false;
	}

	return true;
}

static const luaL_Reg lualibs[] = {
	{"", luaopen_base},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_TABLIBNAME, luaopen_table},
	{LUA_IOLIBNAME, luaopen_io},
	{LUA_OSLIBNAME, luaopen_os},
	{LUA_LOADLIBNAME, luaopen_package},
	{LUA_DBLIBNAME, luaopen_debug},
#ifdef CSLUA_HAS_BIT
	{LUA_BITLIBNAME, luaopen_bit},
#endif
#ifdef CSLUA_HAS_JIT
	{LUA_JITLIBNAME, luaopen_jit},
#endif
	{"log", luaopen_log},
	{"keys", luaopen_keys},
	{"block", luaopen_block},
	{"world", luaopen_world},
	{"client", luaopen_client},
	{"config", luaopen_config},
	{"command", luaopen_command},
	{"vector", luaopen_vector},
	{"angle", luaopen_angle},
	{"color", luaopen_color},
	{"groups", luaopen_groups},
	{"model", luaopen_model},
	{"survival", luaopen_survival},

	{NULL,NULL}
};

LuaScript *lua_getscript(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RSCPTR);
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

// static int sleepmillis(lua_State *L) {
// 	lua_Integer ms = luaL_checkinteger(L, 1);
// 	luaL_argcheck(L, ms > 0, 1, "Sleep timeout must be greater than zero");
// 	Thread_Sleep((cs_uint32)ms);
// 	return 0;
// }

static int mstime(lua_State *L) {
	lua_pushnumber(L, Time_GetMSecD());
	return 1;
}

static int ioensure(lua_State *L) {
	cs_str path = luaL_checkstring(L, 1);
	lua_pushboolean(L, Directory_Ensure(path));
	return 1;
}

static int ioscrname(lua_State *L) {
	LuaScript *script = lua_getscript(L);
	cs_str ext = String_LastChar(script->scrname, '.');
	lua_pushlstring(L, script->scrname, ext - script->scrname);
	return 1;
}

static int iodatafolder(lua_State *L) {
	lua_pushstring(L, "luadata" PATH_DELIM);
	ioscrname(L);
	lua_concat(L, 2);
	return 1;
}

static cs_str const iodel[] = {
	"input", "output", "stdout",
	"stderr", "stdin", "read",
	"write", NULL
};

static cs_str const osdel[] = {
	"setlocale", "execute", "exit",
	"getenv", NULL
};

static const luaL_Reg iofuncs[] = {
	{"ensure", ioensure},
	{"datafolder", iodatafolder},
	{"scrname", ioscrname},

	{NULL, NULL}
};

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
		lua_setfield(script->L, LUA_REGISTRYINDEX, CSLUA_RSCPTR);

		lua_pushcfunction(script->L, allowhotreload);
		lua_setglobal(script->L, "allowHotReload");

		// lua_pushcfunction(script->L, sleepmillis);
		// lua_setglobal(script->L, "sleepMs");

		lua_pushcfunction(script->L, mstime);
		lua_setglobal(script->L, "msTime");

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

#		ifdef CSLUA_HAS_JIT
			luaL_findtable(script->L, LUA_REGISTRYINDEX, "_PRELOAD", 1);
			lua_pushcfunction(script->L, luaopen_ffi);
			lua_setfield(script->L, -2, LUA_FFILIBNAME);
			lua_pop(script->L, 1);
#		endif

		luainit_cuboid(script->L);

		if(LuaScript_GlobalLookup(script, LUA_IOLIBNAME)) {
			for(cs_int32 i = 0; iodel[i]; i++) {
				lua_pushnil(script->L);
				lua_setfield(script->L, -2, iodel[i]);
			}
			luaL_setfuncs(script->L, iofuncs, 0);
			lua_pop(script->L, 1);
		}

		if(LuaScript_GlobalLookup(script, LUA_OSLIBNAME)) {
			for(cs_int32 i = 0; osdel[i]; i++) {
				lua_pushnil(script->L);
				lua_setfield(script->L, -2, osdel[i]);
			}
			lua_pop(script->L, 1);
		}

#		ifdef CORE_USE_UNIX
			lua_getglobal(script->L, "package");
			lua_getfield(script->L, -1, "path");
			lua_pushstring(script->L, ";./lua/?.lua;./lua/?/init.lua");
			lua_concat(script->L, 2);
			lua_setfield(script->L, -2, "path");
			lua_pop(script->L, 1);
#		endif

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
