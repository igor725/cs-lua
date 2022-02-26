#ifndef CSLUAPLUGIN_H
#define CSLUAPLUGIN_H
#ifdef CORE_USE_WINDOWS
#define LUA_BUILD_AS_DLL
#define LUA_LIB
#endif
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <core.h>
#include <platform.h>

typedef struct LuaScript {
	cs_bool hotreload, unloaded;
	cs_str scrname;
	lua_State *L;
	Mutex *lock;
} LuaScript;

#if LUA_VERSION_NUM > 501
#define luaL_register(L, n, lib) luaL_newlib(L, lib)
#define lua_objlen(L, idx) lua_rawlen(L, idx)
#endif

#define lua_addnumconst(L, n) lua_pushnumber(L, n); lua_setglobal(L, #n);
#define LuaScript_Lock(p) Mutex_Lock((p)->lock)
#define LuaScript_Unlock(p) Mutex_Unlock((p)->lock)
#define LuaScript_PrintError(p) Log_Info("lua_State(%p) got an error: %s", (p)->L, lua_tostring((p)->L, -1))

LuaScript *lua_getplugin(lua_State *L);
LuaScript *LuaScript_Open(cs_str name);
cs_bool LuaScript_GlobalLookup(LuaScript *plugin, cs_str key);
cs_bool LuaScript_RegistryLookup(LuaScript *plugin, cs_str regtab, cs_str key);
cs_bool LuaScript_Call(LuaScript *plugin, int args, int ret);
cs_bool LuaScript_Reload(LuaScript *plugin);
cs_bool LuaScript_Close(LuaScript *plugin);
#endif
