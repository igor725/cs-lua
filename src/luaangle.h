#ifndef CSLUAANGLE_H
#define CSLUAANGLE_H
#include <core.h>
#include <vector.h>
#include "scripting.h"

cs_bool scr_isangle(lua_State *L, int idx);
Ang *scr_newangle(lua_State *L);
Ang *scr_checkangle(lua_State *L, int idx);
Ang *scr_toangle(lua_State *L, int idx);

int scr_libfunc(angle)(lua_State *L);
#endif
