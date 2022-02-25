#ifndef CSLUACONFIG_H
#define CSLUACONFIG_H
#include <core.h>
#include <types/config.h>
#include "luaplugin.h"

CStore *lua_checkcfgstore(lua_State *L, int idx);
int luaopen_config(lua_State *L);
#endif
