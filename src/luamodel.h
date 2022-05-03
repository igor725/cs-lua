#ifndef CSLUAMODEL_H
#define CSLUAMODEL_H
#include <types/cpe.h>
#include <lua.h>

CPEModel *lua_checkmodel(lua_State *L, int idx);
int luaopen_model(lua_State *L);
#endif
