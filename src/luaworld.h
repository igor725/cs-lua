#ifndef CSLUAWORLD_H
#define CSLUAWORLD_H
#include <types/world.h>
#include "scripting.h"

World *scr_checkworld(scr_Context *L, int idx);
World *scr_toworld(scr_Context *L, int idx);
void scr_pushworld(scr_Context *L, World *world);
void scr_clearworld(scr_Context *L, World *world);

int scr_libfunc(world)(scr_Context *L);
#endif
