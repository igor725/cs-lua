#ifndef CSLUAPLUGIN_H
#define CSLUAPLUGIN_H
#define LUA_BUILD_AS_DLL
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <core.h>

typedef struct LuaPlugin {
	cs_bool hotreload, unloaded;
	cs_str scrname;
	lua_State *L;
	Mutex *lock;
} LuaPlugin;

#define lua_addnumconst(L, n) lua_pushnumber(L, n); lua_setglobal(L, #n);
#define LuaPlugin_Lock(p) Mutex_Lock((p)->lock)
#define LuaPlugin_Unlock(p) Mutex_Unlock((p)->lock)
#define LuaPlugin_PrintError(p) Log_Info("lua_State(%p) got an error: %s", (p)->L, lua_tostring((p)->L, -1))

LuaPlugin *LuaPlugin_Open(cs_str name);
cs_bool LuaPlugin_GlobalLookup(LuaPlugin *plugin, cs_str key);
cs_bool LuaPlugin_Call(LuaPlugin *plugin, int args, int ret);
cs_bool LuaPlugin_Reload(LuaPlugin *plugin);
cs_bool LuaPlugin_Close(LuaPlugin *plugin);
#endif
