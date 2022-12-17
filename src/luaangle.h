#ifndef CSLUAANGLE_H
#define CSLUAANGLE_H
#include <core.h>
#include <vector.h>
#include "scripting.h"

cs_bool lua_isangle(scr_Context *L, int idx);
Ang *lua_newangle(scr_Context *L);
Ang *lua_checkangle(scr_Context *L, int idx);
Ang *lua_toangle(scr_Context *L, int idx);
int luaopen_angle(scr_Context *L);
#endif
