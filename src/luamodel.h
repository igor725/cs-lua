#ifndef CSLUAMODEL_H
#define CSLUAMODEL_H
#include <types/cpe.h>
#include "scripting.h"

CPEModel *lua_checkmodel(scr_Context *L, int idx);
int luaopen_model(scr_Context *L);
#endif
