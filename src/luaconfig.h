#ifndef CSLUACONFIG_H
#define CSLUACONFIG_H
#include <types/config.h>
#include "scripting.h"

CStore *lua_checkcfgstore(scr_Context *L, int idx);
int luaopen_config(scr_Context *L);
#endif
