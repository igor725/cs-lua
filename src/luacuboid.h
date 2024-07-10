#ifndef CSLUASELECTION_H
#define CSLUASELECTION_H
#include <types/client.h>
#include <types/cpe.h>
#include "scripting.h"

void scr_newcubref(lua_State *L, Client *client, CPECuboid *cub);
void scr_clearcuboids(lua_State *L, Client *client);
void luainit_cuboid(lua_State *L);
#endif
