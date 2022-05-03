#ifndef CSLUAWORLD_H
#define CSLUAWORLD_H
#include <types/world.h>
#include <lua.h>

World *lua_checkworld(lua_State *L, int idx);
World *lua_toworld(lua_State *L, int idx);
void lua_pushworld(lua_State *L, World *world);
void lua_clearworld(lua_State *L, World *world);
int luaopen_world(lua_State *L);
#endif
