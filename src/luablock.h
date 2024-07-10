#ifndef LUABLOCK_H
#define LUABLOCK_H
#include <types/block.h>
#include <types/cpe.h>
#include "scripting.h"

BlockDef *scr_checkblockdef(lua_State *L, int idx);
BulkBlockUpdate *scr_checkbulk(lua_State *L, int idx);

int scr_libfunc(block)(lua_State *L);
#endif
