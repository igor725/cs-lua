#ifndef CSLUAWORLD_H
#define CSLUAWORLD_H
#include <types/world.h>
#include "scripting.h"

World *scr_checkworld(lua_State *L, int idx);
World *scr_toworld(lua_State *L, int idx);
void scr_pushworld(lua_State *L, World *world);
void scr_clearworld(lua_State *L, World *world);

int scr_libfunc(world)(lua_State *L);
#endif
