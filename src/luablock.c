#include <core.h>
#include <block.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luaworld.h"
#include "luablock.h"

static int block_define(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	
	return 0;
}

static const luaL_Reg blocklib[] = {
	{"define", block_define},

	{NULL, NULL}
};

int luaopen_block(lua_State *L) {
	luaL_register(L, luaL_checkstring(L, 1), blocklib);
	return 1;
}
