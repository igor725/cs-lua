#ifndef LUABLOCK_H
#define LUABLOCK_H
#include <types/block.h>
#include <types/cpe.h>
#include "scripting.h"

BlockDef *lua_checkblockdef(scr_Context *L, int idx);
BulkBlockUpdate *lua_checkbulk(scr_Context *L, int idx);
int luaopen_block(scr_Context *L);
#endif
