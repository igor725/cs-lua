#include <core.h>
#include <str.h>
#include <list.h>
#include <world.h>
#include <client.h>
#include <generators.h>
#include "luascript.h"
#include "luaworld.h"
#include "luavector.h"
#include "luaangle.h"
#include "luacolor.h"
#include "luaclient.h"

World *scr_checkworld(scr_Context *L, int idx) {
	void **ud = scr_checkmemtype(L, idx, CSSCRIPTS_MWORLD);
	scr_argassert(L, *ud != NULL, idx, "Invalid world");
	return (World *)*ud;
}

World *scr_toworld(scr_Context *L, int idx) {
	void **ud = scr_testmemtype(L, idx, CSSCRIPTS_MWORLD);
	return ud ? *ud : NULL;
}

void scr_pushworld(scr_Context *L, World *world) {
	if(!world) {
		scr_pushnull(L);
		return;
	}

	cs_str name = World_GetName(world);
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RWORLDS);
	scr_gettabfield(L, -1, name);
	if(scr_isnull(L, -1)) {
		scr_stackpop(L, 1);
		scr_allocmem(L, sizeof(World *));
		scr_setmemtype(L, CSSCRIPTS_MWORLD);
		scr_settabfield(L, -2, name);
		scr_gettabfield(L, -1, name);
	}

	scr_stackrem(L, -2);
	void **ud = scr_getmem(L, -1);
	*ud = world;
}

void scr_clearworld(scr_Context *L, World *world) {
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RWORLDS);
	scr_gettabfield(L, -1, World_GetName(world));

	scr_pushnull(L);
	scr_settabfield(L, -3, World_GetName(world)); // CSSCRIPTS_RWORLDS[wname] = nil;

	if(!scr_ismem(L, -1)) {
		scr_stackpop(L, 1);
		return;
	}

	void **ud = scr_getmem(L, -1);
	*ud = NULL;
}

static int meta_getname(scr_Context *L) {
	scr_pushstring(L, World_GetName(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_getspawn(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	Vec *svec = scr_tofloatvector(L, 2);
	Ang *sang = scr_toangle(L, 3);

	if(!svec && scr_isempty(L, 2)) {
		LuaVector *lvec = scr_newvector(L);
		lvec->type = LUAVECTOR_TFLOAT;
		svec = &lvec->value.f;
	}

	if(!sang && scr_isempty(L, 3))
		sang = scr_newangle(L);

	World_GetSpawn(world, svec, sang);
	return 2;
}

static int meta_getoffset(scr_Context *L) {
	cs_uint32 offset = World_GetOffset(
		scr_checkworld(L, 1),
		scr_checkshortvector(L, 2)
	);
	scr_pushinteger(L,
		offset != WORLD_INVALID_OFFSET ? (scr_Integer)offset : -1
	);
	return 1;
}

static int meta_getdimensions(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	SVec *dvec = scr_toshortvector(L, 2);

	if(!dvec) {
		LuaVector *lvec = scr_newvector(L);
		lvec->type = LUAVECTOR_TSHORT;
		dvec = &lvec->value.s;
	}

	World_GetDimensions(world, dvec);
	return 1;
}

static int meta_getblock(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	scr_pushinteger(L, World_GetBlock(world,
		scr_checkshortvector(L, 2)
	));
	return 1;
}

static int meta_getenvcolor(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	EColor ctype = (EColor)scr_checkinteger(L, 2);
	Color3 *col = scr_tocolor3(L, 3);

	if(!col) {
		LuaColor *lcol = scr_newcolor(L);
		lcol->hasAlpha = false;
		col = &lcol->value.c3;
	}

	if(!World_GetEnvColor(world, ctype, col))
		scr_fmterror(L, "Invalid color type specified");

	return 1;
}

static int meta_getenvprop(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)World_GetEnvProp(
		scr_checkworld(L, 1),
		(EProp)scr_checkinteger(L, 2)
	));
	return 1;
}

static int meta_getweather(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)World_GetWeather(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_gettexpack(scr_Context *L) {
	scr_pushstring(L, World_GetTexturePack(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_getaddr(scr_Context *L) {
	cs_uint32 size;
	scr_pushptr(L, World_GetBlockArray(
		scr_checkworld(L, 1), &size
	));
	scr_pushinteger(L, (scr_Integer)size);
	return 2;
}

static int meta_getseed(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)World_GetSeed(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_getplayercount(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)World_CountPlayers(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_isinmemory(scr_Context *L) {
	scr_pushboolean(L, World_IsInMemory(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_ismodified(scr_Context *L) {
	scr_pushboolean(L, World_IsModified(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_setspawn(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	Vec *svec = NULL; Ang *sang = NULL;
	if(!scr_isnull(L, 2)) svec = scr_checkfloatvector(L, 2);
	if(!scr_isnull(L, 3)) sang = scr_checkangle(L, 3);
	World_SetSpawn(world, svec, sang);
	return 0;
}

static int meta_setblock(scr_Context *L) {
	scr_pushboolean(L, World_SetBlock(
		scr_checkworld(L, 1),
		scr_checkshortvector(L, 2),
		(BlockID)scr_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setblocknat(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	SVec *pos = scr_checkshortvector(L, 2);
	BlockID id = (BlockID)scr_checkinteger(L, 3);
	if(World_SetBlock(world, pos, id)) {
		for(ClientID i = 0; i < MAX_CLIENTS; i++) {
			Client *client = Clients_List[i];
			if(!client || !Client_IsInWorld(client, world)) continue;
			Client_SetBlock(client, pos, id);
		}
		scr_pushboolean(L, 1);
	} else scr_pushboolean(L, 0);

	return 1;
}

static int meta_setenvcolor(scr_Context *L) {
	scr_pushboolean(L, World_SetEnvColor(
		scr_checkworld(L, 1),
		(EColor)scr_checkinteger(L, 2),
		scr_checkcolor3(L, 3)
	));
	return 1;
}

static int meta_setenvprop(scr_Context *L) {
	scr_pushboolean(L, World_SetEnvProp(
		scr_checkworld(L, 1),
		(EProp)scr_checkinteger(L, 2),
		(cs_int32)scr_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setweather(scr_Context *L) {
	scr_pushboolean(L, World_SetWeather(
		scr_checkworld(L, 1),
		(cs_int32)scr_checkinteger(L, 2)
	));
	return 1;
}

static int meta_settexpack(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	cs_str texpack = (cs_str)scr_checkstring(L, 2);
	scr_argassert(L, String_Length(texpack) < 64, 2, "URL too long");
	scr_pushboolean(L, World_SetTexturePack(world, texpack));
	return 1;
}

static int meta_setinmemory(scr_Context *L) {
	World_SetInMemory(
		scr_checkworld(L, 1),
		scr_toboolean(L, 2)
	);
	return 0;
}

static int meta_setignoremod(scr_Context *L) {
	World_SetIgnoreModifications(
		scr_checkworld(L, 1),
		scr_toboolean(L, 2)
	);
	return 0;
}

static int meta_isready(scr_Context *L) {
	scr_pushboolean(L, World_IsReadyToPlay(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_haserror(scr_Context *L) {
	scr_pushboolean(L, World_HasError(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_poperror(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	EWorldExtra extra = WORLD_EXTRA_NOINFO;
	EWorldError err = World_PopError(world, &extra);
	scr_pushinteger(L, (scr_Integer)err);
	scr_pushinteger(L, (scr_Integer)extra);
	return 2;
}

static int meta_update(scr_Context *L) {
	scr_pushboolean(L, World_FinishEnvUpdate(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_generate(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	cs_str gname = scr_checkstring(L, 2);
	cs_int32 seed = (cs_int32)scr_optinteger(L, 3, GENERATOR_SEED_FROM_TIME);
	GeneratorRoutine func = Generators_Get(gname);
	if(!func) {
		scr_pushboolean(L, 0);
		scr_pushstring(L, "Invalid generator");
	} else {
		World_Lock(world, 0);
		if(!func(world, seed)) {
			scr_pushboolean(L, 0);
			scr_pushstring(L, "Generator failed");
		} else {
			scr_pushboolean(L, 1);
			World_Unlock(world);
			return 1;
		}
		World_Unlock(world);
	}

	return 2;
}

static int meta_iterplayers(scr_Context *L) {
	World *world = scr_checkworld(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	for(ClientID i = 0; i < MAX_CLIENTS; i++) {
		Client *client = Clients_List[i];
		if(client && Client_IsInWorld(client, world)) {
			scr_stackpush(L, 2);
			scr_pushclient(L, client);
			if(scr_protectedcall(L, 1, 0, 0) != 0) {
				World_Unlock(world);
				scr_error(L);
				return 0;
			}
		}
	}

	return 0;
}

static int meta_remove(scr_Context *L) {
	scr_pushboolean(L, World_Remove(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_unload(scr_Context *L) {
	World_Unload(
		scr_checkworld(L, 1)
	);
	return 0;
}

static int meta_save(scr_Context *L) {
	scr_pushboolean(L, World_Save(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_load(scr_Context *L) {
	scr_pushboolean(L, World_Load(
		scr_checkworld(L, 1)
	));
	return 1;
}

static int meta_lock(scr_Context *L) {
	scr_pushboolean(L, World_Lock(
		scr_checkworld(L, 1),
		(cs_ulong)scr_optinteger(L, 2, 0)
	));
	return 1;
}

static int meta_unlock(scr_Context *L) {
	World_Unlock(
		scr_checkworld(L, 1)
	);
	return 0;
}

static const scr_RegFuncs worldmeta[] = {
	{"getname", meta_getname},
	{"getspawn", meta_getspawn},
	{"getoffset", meta_getoffset},
	{"getdimensions", meta_getdimensions},
	{"getblock", meta_getblock},
	{"getenvcolor", meta_getenvcolor},
	{"getenvprop", meta_getenvprop},
	{"getweather", meta_getweather},
	{"gettexpack", meta_gettexpack},
	{"getaddr", meta_getaddr},
	{"getseed", meta_getseed},
	{"getplayercount", meta_getplayercount},

	{"setspawn", meta_setspawn},
	{"setblock", meta_setblock},
	{"setblocknat", meta_setblocknat},
	{"setenvcolor", meta_setenvcolor},
	{"setenvprop", meta_setenvprop},
	{"setweather", meta_setweather},
	{"settexpack", meta_settexpack},
	{"setinmemory", meta_setinmemory},
	{"setignoremod", meta_setignoremod},

	{"isready", meta_isready},
	{"isinmemory", meta_isinmemory},
	{"ismodified", meta_ismodified},

	{"haserror", meta_haserror},
	{"poperror", meta_poperror},

	{"lock", meta_lock},
	{"unlock", meta_unlock},

	{"update", meta_update},
	{"generate", meta_generate},
	{"iterplayers", meta_iterplayers},

	{"remove", meta_remove},
	{"unload", meta_unload},
	{"save", meta_save},
	{"load", meta_load},

	{NULL, NULL}
};

static int world_create(scr_Context *L) {
	cs_str wname = scr_checkstring(L, 1);
	SVec *dims = scr_checkshortvector(L, 2);
	scr_argassert(L,
		!Vec_IsNegative(*dims) && !Vec_HaveZero(*dims),
		2, "Invalid Vector passed"
	);
	World *world = World_Create(wname);
	if(!World_SetDimensions(world, dims)) {
		World_Free(world);
		scr_fmterror(L, "World is too big");
	}
	World_AllocBlockArray(world);
	if(!World_IsReadyToPlay(world)) {
		World_Free(world);
		scr_fmterror(L, "Failed to create the world");
	}
	World_Add(world);
	scr_pushworld(L, world);
	return 1;
}

static int world_getname(scr_Context *L) {
	scr_pushworld(L, World_GetByName(
		scr_checkstring(L, 1)
	));
	return 1;
}

static int world_iterall(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);

	AListField *tmp;
	List_Iter(tmp, World_Head) {
		World *world = AList_GetValue(tmp).ptr;
		scr_stackpush(L, 1);
		scr_pushworld(L, world);
		if(scr_protectedcall(L, 1, 0, 0) != 0) {
			scr_error(L);
			return 0;
		}
	}

	return 0;
}

static int world_getmain(scr_Context *L) {
	scr_pushworld(L, World_Main);
	return 1;
}

static int world_setmain(scr_Context *L) {
	World_Main = scr_checkworld(L, 1);
	return 0;
}

static const scr_RegFuncs worldlib[] = {
	{"create", world_create},
	{"getbyname", world_getname},
	{"iterall", world_iterall},
	{"getmain", world_getmain},
	{"setmain", world_setmain},

	{NULL, NULL}
};

int scr_libfunc(world)(scr_Context *L) {
	scr_newtable(L);
	scr_settabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RWORLDS);
	scr_createtype(L, CSSCRIPTS_MWORLD, worldmeta);

	scr_addintconst(L, CPE_WMODVAL_NONE);
	scr_addintconst(L, CPE_WMODVAL_COLORS);
	scr_addintconst(L, CPE_WMODVAL_PROPS);
	scr_addintconst(L, CPE_WMODVAL_TEXPACK);
	scr_addintconst(L, CPE_WMODVAL_WEATHER);

	scr_addintconst(L, CPE_WMODCOL_NONE);
	scr_addintconst(L, CPE_WMODCOL_SKY);
	scr_addintconst(L, CPE_WMODCOL_CLOUD);
	scr_addintconst(L, CPE_WMODCOL_FOG);
	scr_addintconst(L, CPE_WMODCOL_AMBIENT);
	scr_addintconst(L, CPE_WMODCOL_DIFFUSE);
	scr_addintconst(L, CPE_WMODCOL_SKYBOX);

	scr_addintconst(L, CPE_WMODPROP_NONE);
	scr_addintconst(L, CPE_WMODPROP_EDGEID);
	scr_addintconst(L, CPE_WMODPROP_EDGEHEIGHT);
	scr_addintconst(L, CPE_WMODPROP_CLOUDSHEIGHT);
	scr_addintconst(L, CPE_WMODPROP_FOGDISTANCE);
	scr_addintconst(L, CPE_WMODPROP_CLOUDSSPEED);
	scr_addintconst(L, CPE_WMODPROP_WEATHERSPEED);
	scr_addintconst(L, CPE_WMODPROP_WEATHERFADE);
	scr_addintconst(L, CPE_WMODPROP_EXPONENTIALFOG);
	scr_addintconst(L, CPE_WMODPROP_MAPEDGEHEIGHT);

	scr_addintconst(L, WORLD_COLOR_SKY);
	scr_addintconst(L, WORLD_COLOR_CLOUD);
	scr_addintconst(L, WORLD_COLOR_FOG);
	scr_addintconst(L, WORLD_COLOR_AMBIENT);
	scr_addintconst(L, WORLD_COLOR_DIFFUSE);

	scr_addintconst(L, WORLD_PROP_SIDEBLOCK);
	scr_addintconst(L, WORLD_PROP_EDGEBLOCK);
	scr_addintconst(L, WORLD_PROP_EDGELEVEL);
	scr_addintconst(L, WORLD_PROP_CLOUDSLEVEL);
	scr_addintconst(L, WORLD_PROP_FOGDIST);
	scr_addintconst(L, WORLD_PROP_SPDCLOUDS);
	scr_addintconst(L, WORLD_PROP_SPDWEATHER);
	scr_addintconst(L, WORLD_PROP_FADEWEATHER);
	scr_addintconst(L, WORLD_PROP_EXPFOG);
	scr_addintconst(L, WORLD_PROP_SIDEOFFSET);

	scr_newlib(L, worldlib);
	return 1;
}
