#ifndef LUABLOCK_H
#define LUABLOCK_H
#include <types/block.h>
#include <types/cpe.h>
#include "scripting.h"

BlockDef *scr_checkblockdef(scr_Context *L, int idx);
BulkBlockUpdate *scr_checkbulk(scr_Context *L, int idx);

int scr_libfunc(block)(scr_Context *L);
#endif
