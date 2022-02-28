#include <core.h>
#include <block.h>
#include "luascript.h"
#include "luaclient.h"
#include "luaworld.h"
#include "luablock.h"

BlockDef *lua_checkblockdef(lua_State *L, int idx) {
	return *(BlockDef **)luaL_checkudata(L, idx, CSLUA_MBLOCK);
}

static int meta_addtoworld(lua_State *L) {
	BlockDef *bdef = lua_checkblockdef(L, 1);
	World *world = lua_checkworld(L, 2);
	BlockID id = (BlockID)luaL_checkinteger(L, 3);
	lua_pushboolean(L, Block_Define(world, id, bdef));
	return 1;
}

static int meta_undefine(lua_State *L) {
	BlockDef *bdef = lua_checkblockdef(L, 1);
	World *world = lua_checkworld(L, 2);
	lua_pushboolean(L, Block_Undefine(world, bdef));
	return 1;
}

static int meta_globundefine(lua_State *L) {
	BlockDef *bdef = lua_checkblockdef(L, 1);
	Block_UndefineGlobal(bdef);
	return 0;
}

static int meta_update(lua_State *L) {
	BlockDef *bdef = lua_checkblockdef(L, 1);
	Block_UpdateDefinition(bdef);
	return 0;
}

static int meta_gc(lua_State *L) {
	BlockDef *bdef = lua_checkblockdef(L, 1);
	Block_UndefineGlobal(bdef);
	Block_UpdateDefinition(bdef);
	Block_Free(bdef);
	return 0;
}

static const luaL_Reg blockmeta[] = {
	{"addtoworld", meta_addtoworld},
	{"undefine", meta_undefine},
	{"globundefine", meta_globundefine},
	{"update", meta_update},
	{"__gc", meta_gc},

	{NULL, NULL}
};

#define readtabval(I, K, S, T, D) \
lua_getfield(L, -I, K); \
bdef->params.S = (T)luaL_optinteger(L, -1, D)

#define readtabbool(I, K, S) \
lua_getfield(L, -I, K); \
bdef->params.S = (cs_bool)lua_toboolean(L, -1);

// TODO: Использовать SVec для установки минимальных и максимальных значений
// TODO2: Использовать Color3 для установки цвета тумана

static void ReadExtendedBlockTable(lua_State *L, BlockDef *bdef) {
	readtabval(1, "solidity", ext.solidity, EBlockSolidity, BDSOL_SOLID);
	readtabval(2, "movespeed", ext.moveSpeed, cs_byte, 128);
	readtabval(3, "toptex", ext.topTex, cs_byte, 0);
	readtabval(4, "lefttex", ext.leftTex, cs_byte, 0);
	readtabval(5, "righttex", ext.rightTex, cs_byte, 0);
	readtabval(6, "fronttex", ext.frontTex, cs_byte, 0);
	readtabval(7, "backtex", ext.backTex, cs_byte, 0);
	readtabval(8, "bottomtex", ext.bottomTex, cs_byte, 0);
	readtabbool(9, "transmitslight", ext.transmitsLight);
	readtabval(10, "walksound", ext.walkSound, EBlockSounds, BDSND_NONE);
	readtabbool(11, "fullbright", ext.fullBright);
	readtabval(12, "minx", ext.minX, cs_byte, 0);
	readtabval(13, "miny", ext.minY, cs_byte, 0);
	readtabval(14, "minz", ext.minZ, cs_byte, 0);
	readtabval(15, "maxx", ext.maxX, cs_byte, 16);
	readtabval(16, "maxy", ext.maxY, cs_byte, 16);
	readtabval(17, "maxz", ext.maxZ, cs_byte, 16);
	readtabval(18, "drawtype", ext.blockDraw, EBlockDrawTypes, BDDRW_OPAQUE);
	readtabval(19, "fogdensity", ext.fogDensity, cs_byte, 0);
	readtabval(20, "fogr", ext.fogR, cs_byte, 0);
	readtabval(21, "fogg", ext.fogG, cs_byte, 0);
	readtabval(22, "fogb", ext.fogB, cs_byte, 0);
	lua_pop(L, 22);
}

static void ReadBlockTable(lua_State *L, BlockDef *bdef) {
	readtabval(1, "solidity", nonext.solidity, EBlockSolidity, BDSOL_SOLID);
	readtabval(2, "movespeed", nonext.moveSpeed, cs_byte, 128);
	readtabval(3, "toptex", nonext.topTex, cs_byte, 0);
	readtabval(4, "sidetex", nonext.sideTex, cs_byte, 0);
	readtabval(5, "bottomtex", nonext.bottomTex, cs_byte, 0);
	readtabbool(6, "transmitslight", nonext.transmitsLight);
	readtabval(7, "walksound", nonext.walkSound, EBlockSounds, BDSND_NONE);
	readtabbool(8, "fullbright", nonext.fullBright);
	readtabval(9, "shape", nonext.shape, cs_byte, 16);
	readtabval(10, "drawtype", nonext.blockDraw, EBlockDrawTypes, BDDRW_OPAQUE);
	readtabval(11, "fogdensity", nonext.fogDensity, cs_byte, 0);
	readtabval(12, "fogr", nonext.fogR, cs_byte, 0);
	readtabval(13, "fogg", nonext.fogG, cs_byte, 0);
	readtabval(14, "fogb", nonext.fogB, cs_byte, 0);
	lua_pop(L, 14);
}

static int block_define(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, -1, "name");
	cs_str name = luaL_checkstring(L, -1);
	lua_getfield(L, -2, "extended");
	lua_getfield(L, -3, "fallback");
	BlockID fallback = (BlockID)luaL_checkinteger(L, -1);
	lua_getfield(L, -4, "params");
	BlockDef *bdef = NULL;

	if(lua_toboolean(L, -3)) {
		bdef = Block_New(name, BDF_EXTENDED);
		ReadExtendedBlockTable(L, bdef);
	} else {
		bdef = Block_New(name, 0);
		ReadBlockTable(L, bdef);
	}

	lua_pop(L, 3);
	void **ud = lua_newuserdata(L, sizeof(BlockDef *));
	luaL_setmetatable(L, CSLUA_MBLOCK);
	bdef->fallback = fallback;
	*ud = bdef;
	return 1;
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
	luaL_newmetatable(L, CSLUA_MBLOCK);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, blockmeta, 0);
	lua_pop(L, 1);

	lua_addnumconst(L, BDSOL_WALK);
	lua_addnumconst(L, BDSOL_SWIM);
	lua_addnumconst(L, BDSOL_SOLID);
	
	lua_addnumconst(L, BDSND_NONE);
	lua_addnumconst(L, BDSND_WOOD);
	lua_addnumconst(L, BDSND_GRAVEL);
	lua_addnumconst(L, BDSND_GRASS);
	lua_addnumconst(L, BDSND_STONE);
	lua_addnumconst(L, BDSND_METAL);
	lua_addnumconst(L, BDSND_GLASS);
	lua_addnumconst(L, BDSND_WOOL);
	lua_addnumconst(L, BDSND_SAND);
	lua_addnumconst(L, BDSND_SNOW);

	lua_addnumconst(L, BDDRW_OPAQUE);
	lua_addnumconst(L, BDDRW_TRANSPARENT);
	lua_addnumconst(L, BDDRW_TRANSPARENT2);
	lua_addnumconst(L, BDDRW_TRANSLUCENT);
	lua_addnumconst(L, BDDRW_GAS);

	luaL_register(L, luaL_checkstring(L, 1), blocklib);
	return 1;
}
