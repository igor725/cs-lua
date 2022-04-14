#ifndef CSLUASELECTION_H
#define CSLUASELECTION_H
#include <core.h>
#include <types/cpe.h>
#include "luascript.h"

void lua_newcubref(lua_State *L, Client *client, CPECuboid *cub);
void lua_clearcuboids(lua_State *L, Client *client);
void luainit_cuboid(lua_State *L);
#endif
