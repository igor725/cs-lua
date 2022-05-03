#ifndef LUABLOCK_H
#define LUABLOCK_H
#include <types/block.h>
#include <types/cpe.h>
#include <lua.h>

BlockDef *lua_checkblockdef(lua_State *L, int idx);
BulkBlockUpdate *lua_checkbulk(lua_State *L, int idx);
int luaopen_block(lua_State *L);
#endif
