#ifndef LUABLOCK_H
#define LUABLOCK_H
#include <core.h>
#include "luascript.h"

BlockDef *lua_checkblockdef(lua_State *L, int idx);
int luaopen_block(lua_State *L);
#endif
