#ifndef CSLUACONFIG_H
#define CSLUACONFIG_H
#include <types/config.h>
#include <lua.h>

CStore *lua_checkcfgstore(lua_State *L, int idx);
int luaopen_config(lua_State *L);
#endif
