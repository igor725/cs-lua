#include <core.h>
#include <block.h>
#include <platform.h>
#include <str.h>
#include "luascript.h"
#include "luaworld.h"
#include "luablock.h"

BlockDef *scr_checkblockdef(scr_Context *L, int idx) {
	return scr_checkmemtype(L, idx, CSSCRIPTS_MBLOCK);
}

BulkBlockUpdate *scr_checkbulk(scr_Context *L, int idx) {
	return scr_checkmemtype(L, idx, CSSCRIPTS_MBULK);
}

static int meta_addtoworld(scr_Context *L) {
	scr_pushboolean(L, Block_Define(
		scr_checkworld(L, 2),
		(BlockID)scr_checkinteger(L, 3),
		scr_checkblockdef(L, 1)
	));
	return 1;
}

static int meta_undefine(scr_Context *L) {
	scr_pushboolean(L, Block_Undefine(
		scr_checkworld(L, 2),
		scr_checkblockdef(L, 1)
	));
	return 1;
}

static int meta_globundefine(scr_Context *L) {
	Block_UndefineGlobal(
		scr_checkblockdef(L, 1)
	);
	return 0;
}

static int meta_update(scr_Context *L) {
	Block_UpdateDefinition(
		scr_checkblockdef(L, 1)
	);
	return 0;
}

static int meta_gc(scr_Context *L) {
	BlockDef *bdef = scr_checkblockdef(L, 1);
	Block_UndefineGlobal(bdef);
	Block_UpdateDefinition(bdef);
	return 0;
}

static const scr_RegFuncs blockmeta[] = {
	{"addtoworld", meta_addtoworld},
	{"undefine", meta_undefine},
	{"globundefine", meta_globundefine},
	{"update", meta_update},

	{"__gc", meta_gc},

	{NULL, NULL}
};

static int meta_setautosend(scr_Context *L) {
	BulkBlockUpdate *bbu = scr_checkbulk(L, 1);
	bbu->autosend = scr_toboolean(L, 2);
	return 0;
}

static int meta_setworld(scr_Context *L) {
	BulkBlockUpdate *bbu = scr_checkbulk(L, 1);
	bbu->world = scr_checkworld(L, 2);
	return 0;
}

static int meta_getworld(scr_Context *L) {
	BulkBlockUpdate *bbu = scr_checkbulk(L, 1);
	scr_pushworld(L, bbu->world);
	return 1;
}

static int meta_push(scr_Context *L) {
	BulkBlockUpdate *bbu = scr_checkbulk(L, 1);
	scr_pushboolean(L, Block_BulkUpdateSend(bbu));
	return 1;
}

static int meta_add(scr_Context *L) {
	BulkBlockUpdate *bbu = scr_checkbulk(L, 1);
	int top;

	if(scr_istable(L, 2)) {
		top = (int)scr_gettablen(L, 2);
		if(top == 0 || (top % 2 != 0))
			scr_fmterror(L, "Invalid table size");
		top /= 2;
		for(int i = 0; i < top; i++) {
			scr_pushinteger(L, (i * 2) + 2);
			scr_getfromtable(L, 2);
			scr_pushinteger(L, (i * 2) + 1);
			scr_getfromtable(L, 2);
			cs_uint32 offset = (cs_uint32)scr_checkinteger(L, -1);
			BlockID block = (BlockID)scr_checkinteger(L, -2);
			scr_stackpop(L, 2);
			if(!Block_BulkUpdateAdd(bbu, offset, block)) {
				scr_pushboolean(L, 0);
				scr_pushinteger(L, i);
				return 2;
			}
		}
	} else {
		top = scr_stacktop(L) - 1;
		if(top == 0 || top % 2 != 0)
			scr_fmterror(L, "Invalid number of arguments");
		top /= 2;
		for(int i = 0; i < top; i++) {
			cs_uint32 offset = (cs_uint32)scr_checkinteger(L, (i * 2) + 2);
			BlockID block = (BlockID)scr_checkinteger(L, (i * 2) + 3);
			if(!Block_BulkUpdateAdd(bbu, offset, block)) {
				scr_pushboolean(L, 0);
				scr_pushinteger(L, i);
				return 2;
			}
		}
	}

	scr_pushboolean(L, 1);
	return 1;
}

static const scr_RegFuncs bulkmeta[] = {
	{"setautosend", meta_setautosend},
	{"setworld", meta_setworld},

	{"getworld", meta_getworld},

	{"push", meta_push},
	{"add", meta_add},

	{NULL, NULL}
};

#define readtabval(K, S, T, D) \
(scr_gettabfield(L, params, K), bdef->params.S = (T)scr_optinteger(L, -1, D))

#define readtabbool(K, S) \
(scr_gettabfield(L, params, K), bdef->params.S = scr_toboolean(L, -1))

// TODO: Использовать SVec для установки минимальных и максимальных значений
// TODO2: Использовать Color3 для установки цвета тумана

static void ReadExtendedBlockTable(scr_Context *L, BlockDef *bdef, int params) {
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
	scr_stackpop(L, 22);
}

static void ReadBlockTable(scr_Context *L, BlockDef *bdef, int params) {
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
	scr_stackpop(L, 14);
}

static int block_define(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	cs_str name = NULL;
	if(scr_checktabfield(L, 1, "name", LUA_TSTRING))
		name = scr_tostring(L, -1);
	scr_gettabfield(L, 1, "extended");
	BlockID fallback = 0;
	if(scr_checktabfield(L, 1, "fallback", LUA_TNUMBER))
		fallback = (BlockID)scr_tointeger(L, -1);
	scr_checktabfield(L, 1, "params", LUA_TTABLE);
	int params = lua_absindex(L, -1);

	BlockDef *bdef = scr_allocmem(L, sizeof(BlockDef));
	scr_setmemtype(L, CSSCRIPTS_MBLOCK);
	String_Copy(bdef->name, MAX_STR_LEN, name);
	bdef->fallback = fallback;

	if(scr_toboolean(L, -4)) {
		bdef->flags = BDF_EXTENDED;
		ReadExtendedBlockTable(L, bdef, params);
	} else {
		bdef->flags = 0;
		ReadBlockTable(L, bdef, params);
	}

	return 1;
}

static int block_isvalid(scr_Context *L) {
	scr_pushboolean(L, Block_IsValid(
		scr_checkworld(L, 1),
		(BlockID)scr_checkinteger(L, 2)
	));
	return 1;
}

static int block_fallbackfor(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Block_GetFallbackFor(
		scr_checkworld(L, 1),
		(BlockID)scr_checkinteger(L, 2)
	));
	return 1;
}

static int block_bulk(scr_Context *L) {
	BulkBlockUpdate *bbu = scr_allocmem(L, sizeof(BulkBlockUpdate));
	Memory_Fill(bbu, sizeof(BulkBlockUpdate), 0);
	scr_setmemtype(L, CSSCRIPTS_MBULK);
	bbu->world = scr_toworld(L, 1);
	bbu->autosend = scr_toboolean(L, 2);
	return 1;
}

static const scr_RegFuncs blocklib[] = {
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

static void initconts(scr_Context *L) {
	for(int i = 0; blocknames[i]; i++) {
		scr_pushinteger(L, i);
		scr_settabfield(L, -2, blocknames[i]);
	}
}

int scr_libfunc(block)(scr_Context *L) {
	scr_createtype(L, CSSCRIPTS_MBLOCK, blockmeta);
	scr_createtype(L, CSSCRIPTS_MBULK, bulkmeta);

	scr_addintconst(L, BDSOL_WALK);
	scr_addintconst(L, BDSOL_SWIM);
	scr_addintconst(L, BDSOL_SOLID);

	scr_addintconst(L, BDSND_NONE);
	scr_addintconst(L, BDSND_WOOD);
	scr_addintconst(L, BDSND_GRAVEL);
	scr_addintconst(L, BDSND_GRASS);
	scr_addintconst(L, BDSND_STONE);
	scr_addintconst(L, BDSND_METAL);
	scr_addintconst(L, BDSND_GLASS);
	scr_addintconst(L, BDSND_WOOL);
	scr_addintconst(L, BDSND_SAND);
	scr_addintconst(L, BDSND_SNOW);

	scr_addintconst(L, BDDRW_OPAQUE);
	scr_addintconst(L, BDDRW_TRANSPARENT);
	scr_addintconst(L, BDDRW_TRANSPARENT2);
	scr_addintconst(L, BDDRW_TRANSLUCENT);
	scr_addintconst(L, BDDRW_GAS);

	scr_newlib(L, blocklib);
	initconts(L);
	return 1;
}
