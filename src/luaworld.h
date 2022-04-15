#ifndef CSLUAWORLD_H
#define CSLUAWORLD_H
#include <core.h>
#include <types/world.h>
#include "luascript.h"

World *lua_checkworld(lua_State *L, int idx);
void lua_pushworld(lua_State *L, World *world);
void lua_clearworld(lua_State *L, World *world);
int luaopen_world(lua_State *L);
#endif
