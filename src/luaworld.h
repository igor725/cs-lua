#ifndef CSLUAWORLD_H
#define CSLUAWORLD_H
#include <types/world.h>
#include "scripting.h"

World *lua_checkworld(scr_Context *L, int idx);
World *lua_toworld(scr_Context *L, int idx);
void lua_pushworld(scr_Context *L, World *world);
void lua_clearworld(scr_Context *L, World *world);
int luaopen_world(scr_Context *L);
#endif
