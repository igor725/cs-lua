#ifndef CSLUAANGLE_H
#define CSLUAANGLE_H
#include <core.h>
#include <vector.h>
#include "luaplugin.h"

Ang *lua_newangle(lua_State *L);
Ang *lua_checkangle(lua_State *L, int idx);
int luaopen_angle(lua_State *L);
#endif
