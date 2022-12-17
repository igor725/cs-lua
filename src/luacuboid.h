#ifndef CSLUASELECTION_H
#define CSLUASELECTION_H
#include <types/client.h>
#include <types/cpe.h>
#include "scripting.h"

void lua_newcubref(scr_Context *L, Client *client, CPECuboid *cub);
void lua_clearcuboids(scr_Context *L, Client *client);
void luainit_cuboid(scr_Context *L);
#endif
