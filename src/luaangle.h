#ifndef CSLUAANGLE_H
#define CSLUAANGLE_H
#include <vector.h>
#include "luascript.h"

cs_bool lua_isangle(lua_State *L, int idx);
Ang *lua_newangle(lua_State *L);
Ang *lua_checkangle(lua_State *L, int idx);
Ang *lua_toangle(lua_State *L, int idx);
int luaopen_angle(lua_State *L);
#endif
