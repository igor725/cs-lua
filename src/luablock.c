#include <core.h>
#include <block.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luaworld.h"
#include "luablock.h"

static const luaL_Reg blockmeta[] = {
	{NULL, NULL}
};

static int block_define(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	
	return 0;
}

static int block_isvalid(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	BlockID id = (BlockID)luaL_checkinteger(L, 2);
	lua_pushboolean(L, Block_IsValid(world, id));
	return 1;
}

static const luaL_Reg blocklib[] = {
	{"define", block_define},
	{"isvalid", block_isvalid},

	{NULL, NULL}
};

int luaopen_block(lua_State *L) {
	luaL_newmetatable(L, "Block");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, blockmeta, 0);
	lua_pop(L, 1);

	luaL_register(L, luaL_checkstring(L, 1), blocklib);
	return 1;
}
