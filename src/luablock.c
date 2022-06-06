#include <core.h>
#include <block.h>
#include <platform.h>
#include <str.h>
#include "luascript.h"
#include "luaworld.h"
#include "luablock.h"

BlockDef *lua_checkblockdef(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, CSLUA_MBLOCK);
}

BulkBlockUpdate *lua_checkbulk(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, CSLUA_MBULK);
}

static int meta_addtoworld(lua_State *L) {
	lua_pushboolean(L, Block_Define(
		lua_checkworld(L, 2),
		(BlockID)luaL_checkinteger(L, 3),
		lua_checkblockdef(L, 1)
	));
	return 1;
}

static int meta_undefine(lua_State *L) {
	lua_pushboolean(L, Block_Undefine(
		lua_checkworld(L, 2),
		lua_checkblockdef(L, 1)
	));
	return 1;
}

static int meta_globundefine(lua_State *L) {
	Block_UndefineGlobal(
		lua_checkblockdef(L, 1)
	);
	return 0;
}

static int meta_update(lua_State *L) {
	Block_UpdateDefinition(
		lua_checkblockdef(L, 1)
	);
	return 0;
}

static int meta_gc(lua_State *L) {
	BlockDef *bdef = lua_checkblockdef(L, 1);
	Block_UndefineGlobal(bdef);
	Block_UpdateDefinition(bdef);
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

static int meta_setautosend(lua_State *L) {
	BulkBlockUpdate *bbu = lua_checkbulk(L, 1);
	bbu->autosend = (cs_bool)lua_toboolean(L, 2);
	return 0;
}

static int meta_setworld(lua_State *L) {
	BulkBlockUpdate *bbu = lua_checkbulk(L, 1);
	bbu->world = lua_checkworld(L, 2);
	return 0;
}

static int meta_getworld(lua_State *L) {
	BulkBlockUpdate *bbu = lua_checkbulk(L, 1);
	lua_pushworld(L, bbu->world);
	return 1;
}

static int meta_push(lua_State *L) {
	BulkBlockUpdate *bbu = lua_checkbulk(L, 1);
	lua_pushboolean(L, Block_BulkUpdateSend(bbu));
	return 1;
}

static int meta_add(lua_State *L) {
	BulkBlockUpdate *bbu = lua_checkbulk(L, 1);
	int top;

	if(lua_istable(L, 2)) {
		top = (int)lua_objlen(L, 2);
		if(top == 0 || (top % 2 != 0))
			luaL_error(L, "Invalid table size");
		top /= 2;
		for(int i = 0; i < top; i++) {
			lua_pushinteger(L, (i * 2) + 2);
			lua_gettable(L, 2);
			lua_pushinteger(L, (i * 2) + 1);
			lua_gettable(L, 2);
			cs_uint32 offset = (cs_uint32)luaL_checkinteger(L, -1);
			BlockID block = (BlockID)luaL_checkinteger(L, -2);
			lua_pop(L, 2);
			if(!Block_BulkUpdateAdd(bbu, offset, block)) {
				lua_pushboolean(L, 0);
				lua_pushinteger(L, i);
				return 2;
			}
		}
	} else {
		top = lua_gettop(L) - 1;
		if(top == 0 || top % 2 != 0)
			luaL_error(L, "Invalid number of arguments");
		top /= 2;
		for(int i = 0; i < top; i++) {
			cs_uint32 offset = (cs_uint32)luaL_checkinteger(L, (i * 2) + 2);
			BlockID block = (BlockID)luaL_checkinteger(L, (i * 2) + 3);
			if(!Block_BulkUpdateAdd(bbu, offset, block)) {
				lua_pushboolean(L, 0);
				lua_pushinteger(L, i);
				return 2;
			}
		}
	}

	lua_pushboolean(L, 1);
	return 1;
}

static const luaL_Reg bulkmeta[] = {
	{"setautosend", meta_setautosend},
	{"setworld", meta_setworld},

	{"getworld", meta_getworld},

	{"push", meta_push},
	{"add", meta_add},

	{NULL, NULL}
};

#define readtabval(K, S, T, D) \
(lua_getfield(L, params, K), bdef->params.S = (T)luaL_optinteger(L, -1, D))

#define readtabbool(K, S) \
(lua_getfield(L, params, K), bdef->params.S = (cs_bool)lua_toboolean(L, -1))

// TODO: Использовать SVec для установки минимальных и максимальных значений
// TODO2: Использовать Color3 для установки цвета тумана

static void ReadExtendedBlockTable(lua_State *L, BlockDef *bdef, int params) {
	readtabval("solidity", ext.solidity, EBlockSolidity, BDSOL_SOLID);
	readtabval("movespeed", ext.moveSpeed, cs_byte, 128);
	readtabval("toptex", ext.topTex, cs_byte, 0);
	readtabval("lefttex", ext.leftTex, cs_byte, 0);
	readtabval("righttex", ext.rightTex, cs_byte, 0);
	readtabval("fronttex", ext.frontTex, cs_byte, 0);
	readtabval("backtex", ext.backTex, cs_byte, 0);
	readtabval("bottomtex", ext.bottomTex, cs_byte, 0);
	readtabbool("transmitslight", ext.transmitsLight);
	readtabval("walksound", ext.walkSound, EBlockSounds, BDSND_NONE);
	readtabbool("fullbright", ext.fullBright);
	readtabval("minx", ext.minX, cs_byte, 0);
	readtabval("miny", ext.minY, cs_byte, 0);
	readtabval("minz", ext.minZ, cs_byte, 0);
	readtabval("maxx", ext.maxX, cs_byte, 16);
	readtabval("maxy", ext.maxY, cs_byte, 16);
	readtabval("maxz", ext.maxZ, cs_byte, 16);
	readtabval("drawtype", ext.blockDraw, EBlockDrawTypes, BDDRW_OPAQUE);
	readtabval("fogdensity", ext.fogDensity, cs_byte, 0);
	readtabval("fogr", ext.fogR, cs_byte, 0);
	readtabval("fogg", ext.fogG, cs_byte, 0);
	readtabval("fogb", ext.fogB, cs_byte, 0);
	lua_pop(L, 22);
}

static void ReadBlockTable(lua_State *L, BlockDef *bdef, int params) {
	readtabval("solidity", nonext.solidity, EBlockSolidity, BDSOL_SOLID);
	readtabval("movespeed", nonext.moveSpeed, cs_byte, 128);
	readtabval("toptex", nonext.topTex, cs_byte, 0);
	readtabval("sidetex", nonext.sideTex, cs_byte, 0);
	readtabval("bottomtex", nonext.bottomTex, cs_byte, 0);
	readtabbool("transmitslight", nonext.transmitsLight);
	readtabval("walksound", nonext.walkSound, EBlockSounds, BDSND_NONE);
	readtabbool("fullbright", nonext.fullBright);
	readtabval("shape", nonext.shape, cs_byte, 16);
	readtabval("drawtype", nonext.blockDraw, EBlockDrawTypes, BDDRW_OPAQUE);
	readtabval("fogdensity", nonext.fogDensity, cs_byte, 0);
	readtabval("fogr", nonext.fogR, cs_byte, 0);
	readtabval("fogg", nonext.fogG, cs_byte, 0);
	readtabval("fogb", nonext.fogB, cs_byte, 0);
	lua_pop(L, 14);
}

static int block_define(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	cs_str name = NULL;
	if(lua_checktabfield(L, 1, "name", LUA_TSTRING))
		name = lua_tostring(L, -1);
	lua_getfield(L, 1, "extended");
	BlockID fallback = 0;
	if(lua_checktabfield(L, 1, "fallback", LUA_TNUMBER))
		fallback = (BlockID)lua_tointeger(L, -1);
	lua_checktabfield(L, 1, "params", LUA_TTABLE);
	int params = lua_absindex(L, -1);

	BlockDef *bdef = lua_newuserdata(L, sizeof(BlockDef));
	luaL_setmetatable(L, CSLUA_MBLOCK);
	String_Copy(bdef->name, MAX_STR_LEN, name);
	bdef->fallback = fallback;

	if(lua_toboolean(L, -4)) {
		bdef->flags = BDF_EXTENDED;
		ReadExtendedBlockTable(L, bdef, params);
	} else {
		bdef->flags = 0;
		ReadBlockTable(L, bdef, params);
	}

	return 1;
}

static int block_isvalid(lua_State *L) {
	lua_pushboolean(L, Block_IsValid(
		lua_checkworld(L, 1),
		(BlockID)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int block_fallbackfor(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)Block_GetFallbackFor(
		lua_checkworld(L, 1),
		(BlockID)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int block_bulk(lua_State *L) {
	BulkBlockUpdate *bbu = lua_newuserdata(L, sizeof(BulkBlockUpdate));
	Memory_Fill(bbu, sizeof(BulkBlockUpdate), 0);
	luaL_setmetatable(L, CSLUA_MBULK);
	bbu->world = lua_toworld(L, 1);
	bbu->autosend = (cs_bool)lua_toboolean(L, 2);
	return 1;
}

static const luaL_Reg blocklib[] = {
	{"define", block_define},
	{"isvalid", block_isvalid},
	{"fallbackfor", block_fallbackfor},
	{"bulk", block_bulk},

	{NULL, NULL}
};

static cs_str const blocknames[] = {
	"AIR", "STONE", "GRASS", "DIRT", "COBBLE",
	"WOOD", "SAPLING", "BEDROCK", "WATER",
	"WATER_STILL", "LAVA", "LAVA_STILL",
	"SAND", "GRAVEL", "GOLD_ORE", "IRON_ORE",
	"COAL_ORE", "LOG", "LEAVES", "SPONGE",
	"GLASS", "RED", "ORANGE", "YELLOW",
	"LIME", "GREEN", "TEAL", "AQUA", "CYAN",
	"BLUE", "INDIGO", "VIOLET", "MAGENTA",
	"PINK", "BLACK", "GRAY", "WHITE",
	"DANDELION", "ROSE", "BROWN_SHROOM",
	"RED_SHROOM", "GOLD", "IRON",
	"DOUBLE_SLAB", "SLAB", "BRICK", "TNT",
	"BOOKSHELF", "MOSSY_ROCKS", "OBSIDIAN",

	"COBBLESLAB", "ROPE", "SANDSTONE", "SNOW",
	"FIRE", "LIGHTPINK", "FORESTGREEN", 
	"BROWN", "DEEPBLUE", "TURQUOISE", "ICE",
	"CERAMICTILE", "MAGMA", "PILLAR", "CRATE",
	"STONEBRICK",

	NULL
};

static void luablock_initconts(lua_State *L) {
	for(int i = 0; blocknames[i]; i++) {
		lua_pushinteger(L, i);
		lua_setfield(L, -2, blocknames[i]);
	}
}

int luaopen_block(lua_State *L) {
	lua_indexedmeta(L, CSLUA_MBLOCK, blockmeta);
	lua_indexedmeta(L, CSLUA_MBULK, bulkmeta);

	lua_addintconst(L, BDSOL_WALK);
	lua_addintconst(L, BDSOL_SWIM);
	lua_addintconst(L, BDSOL_SOLID);

	lua_addintconst(L, BDSND_NONE);
	lua_addintconst(L, BDSND_WOOD);
	lua_addintconst(L, BDSND_GRAVEL);
	lua_addintconst(L, BDSND_GRASS);
	lua_addintconst(L, BDSND_STONE);
	lua_addintconst(L, BDSND_METAL);
	lua_addintconst(L, BDSND_GLASS);
	lua_addintconst(L, BDSND_WOOL);
	lua_addintconst(L, BDSND_SAND);
	lua_addintconst(L, BDSND_SNOW);

	lua_addintconst(L, BDDRW_OPAQUE);
	lua_addintconst(L, BDDRW_TRANSPARENT);
	lua_addintconst(L, BDDRW_TRANSPARENT2);
	lua_addintconst(L, BDDRW_TRANSLUCENT);
	lua_addintconst(L, BDDRW_GAS);

	luaL_newlib(L, blocklib);
	luablock_initconts(L);
	return 1;
}
