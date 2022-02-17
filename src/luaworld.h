#ifndef CSLUAWORLD_H
#define CSLUAWORLD_H
#include <core.h>
#include <world.h>
#include "luaplugin.h"

World *lua_checkworld(lua_State *L, int idx);
void lua_pushworld(lua_State *L, World *client);
int luaopen_world(lua_State *L);
#endif
