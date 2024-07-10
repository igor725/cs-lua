#ifndef CSLUACONFIG_H
#define CSLUACONFIG_H
#include <types/config.h>
#include "scripting.h"

CStore *scr_checkcfgstore(lua_State *L, int idx);

int scr_libfunc(config)(lua_State *L);
#endif
