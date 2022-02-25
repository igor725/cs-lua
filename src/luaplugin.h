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

typedef struct LuaPlugin {
	cs_bool hotreload, unloaded;
	cs_str scrname;
	lua_State *L;
	Mutex *lock;
} LuaPlugin;

#if LUA_VERSION_NUM > 501
#define luaL_register(L, n, lib) luaL_newlib(L, lib)
#define lua_objlen(L, idx) lua_rawlen(L, idx)
#endif

#define lua_addnumconst(L, n) lua_pushnumber(L, n); lua_setglobal(L, #n);
#define LuaPlugin_Lock(p) Mutex_Lock((p)->lock)
#define LuaPlugin_Unlock(p) Mutex_Unlock((p)->lock)
#define LuaPlugin_PrintError(p) Log_Info("lua_State(%p) got an error: %s", (p)->L, lua_tostring((p)->L, -1))

LuaPlugin *lua_getplugin(lua_State *L);
LuaPlugin *LuaPlugin_Open(cs_str name);
cs_bool LuaPlugin_GlobalLookup(LuaPlugin *plugin, cs_str key);
cs_bool LuaPlugin_RegistryLookup(LuaPlugin *plugin, cs_str regtab, cs_str key);
cs_bool LuaPlugin_Call(LuaPlugin *plugin, int args, int ret);
cs_bool LuaPlugin_Reload(LuaPlugin *plugin);
cs_bool LuaPlugin_Close(LuaPlugin *plugin);
#endif
