#ifndef CSLUAMODEL_H
#define CSLUAMODEL_H
#include <types/cpe.h>
#include "scripting.h"

CPEModel *scr_checkmodel(lua_State *L, int idx);

int scr_libfunc(model)(lua_State *L);
#endif
