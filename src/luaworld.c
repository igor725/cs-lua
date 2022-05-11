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

World *lua_checkworld(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, CSLUA_MWORLD);
	luaL_argcheck(L, *ud != NULL, idx, "Invalid world");
	return (World *)*ud;
}

World *lua_toworld(lua_State *L, int idx) {
	void **ud = luaL_testudata(L, idx, CSLUA_MWORLD);
	return ud ? *ud : NULL;
}

void lua_pushworld(lua_State *L, World *world) {
	if(!world) {
		lua_pushnil(L);
		return;
	}

	cs_str name = World_GetName(world);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RWORLDS);
	lua_getfield(L, -1, name);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_newuserdata(L, sizeof(World *));
		luaL_setmetatable(L, CSLUA_MWORLD);
		lua_setfield(L, -2, name);
		lua_getfield(L, -1, name);
	}

	lua_remove(L, -2);
	void **ud = lua_touserdata(L, -1);
	*ud = world;
}

void lua_clearworld(lua_State *L, World *world) {
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RWORLDS);
	lua_pushstring(L, World_GetName(world));
	lua_gettable(L, -2);
	if(!lua_isuserdata(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	void **ud = lua_touserdata(L, -1);
	*ud = NULL;
}

static int meta_getname(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushstring(L, World_GetName(world));
	return 1;
}

static int meta_getspawn(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	Vec *svec = lua_tofloatvector(L, 2);
	Ang *sang = lua_toangle(L, 3);

	if(!svec && lua_isnone(L, 2)) {
		LuaVector *lvec = lua_newvector(L);
		lvec->type = LUAVECTOR_TFLOAT;
		svec = &lvec->value.f;
	}

	if(!sang && lua_isnone(L, 3))
		sang = lua_newangle(L);

	World_GetSpawn(world, svec, sang);
	return 2;
}

static int meta_getoffset(lua_State *L) {
	cs_uint32 offset = World_GetOffset(
		lua_checkworld(L, 1),
		lua_checkshortvector(L, 2)
	);
	lua_pushinteger(L,
		offset != WORLD_INVALID_OFFSET ? (lua_Integer)offset : -1
	);
	return 1;
}

static int meta_getdimensions(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	SVec *dvec = lua_toshortvector(L, 2);

	if(!dvec) {
		LuaVector *lvec = lua_newvector(L);
		lvec->type = LUAVECTOR_TSHORT;
		dvec = &lvec->value.s;
	}

	World_GetDimensions(world, dvec);
	return 1;
}

static int meta_getblock(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushinteger(L, World_GetBlock(world,
		lua_checkshortvector(L, 2)
	));
	return 1;
}

static int meta_getenvcolor(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EColor ctype = (EColor)luaL_checkinteger(L, 2);
	Color3 *col = lua_tocolor3(L, 3);

	if(!col) {
		LuaColor *lcol = lua_newcolor(L);
		lcol->hasAlpha = false;
		col = &lcol->value.c3;
	}

	if(!World_GetEnvColor(world, ctype, col))
		luaL_error(L, "Invalid color type specified");

	return 1;
}

static int meta_getenvprop(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)World_GetEnvProp(
		lua_checkworld(L, 1),
		(EProp)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_getweather(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)World_GetWeather(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_gettexpack(lua_State *L) {
	lua_pushstring(L, World_GetTexturePack(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_getplayercount(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)World_CountPlayers(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_isinmemory(lua_State *L) {
	lua_pushboolean(L, World_IsInMemory(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_ismodified(lua_State *L) {
	lua_pushboolean(L, World_IsModified(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_setspawn(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	Vec *svec = NULL; Ang *sang = NULL;
	if(!lua_isnil(L, 2)) svec = lua_checkfloatvector(L, 2);
	if(!lua_isnil(L, 3)) sang = lua_checkangle(L, 3);
	World_SetSpawn(world, svec, sang);
	return 0;
}

static int meta_setblock(lua_State *L) {
	lua_pushboolean(L, World_SetBlock(
		lua_checkworld(L, 1),
		lua_checkshortvector(L, 2),
		(BlockID)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setblocknat(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	SVec *pos = lua_checkshortvector(L, 2);
	BlockID id = (BlockID)luaL_checkinteger(L, 3);
	if(World_SetBlock(world, pos, id)) {
		for(ClientID i = 0; i < MAX_CLIENTS; i++) {
			Client *client = Clients_List[i];
			if(!client || !Client_IsInWorld(client, world)) continue;
			Client_SetBlock(client, pos, id);
		}
		lua_pushboolean(L, 1);
	} else lua_pushboolean(L, 0);

	return 1;
}

static int meta_setenvcolor(lua_State *L) {
	lua_pushboolean(L, World_SetEnvColor(
		lua_checkworld(L, 1),
		(EColor)luaL_checkinteger(L, 2),
		lua_checkcolor3(L, 3)
	));
	return 1;
}

static int meta_setenvprop(lua_State *L) {
	lua_pushboolean(L, World_SetEnvProp(
		lua_checkworld(L, 1),
		(EProp)luaL_checkinteger(L, 2),
		(cs_int32)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setweather(lua_State *L) {
	lua_pushboolean(L, World_SetWeather(
		lua_checkworld(L, 1),
		(cs_int32)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_settexpack(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	cs_str texpack = (cs_str)luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(texpack) < 64, 2, "URL too long");
	lua_pushboolean(L, World_SetTexturePack(world, texpack));
	return 1;
}

static int meta_setinmemory(lua_State *L) {
	World_SetInMemory(
		lua_checkworld(L, 1),
		(cs_bool)lua_toboolean(L, 2)
	);
	return 0;
}

static int meta_setignoremod(lua_State *L) {
	World_SetIgnoreModifications(
		lua_checkworld(L, 1),
		(cs_bool)lua_toboolean(L, 2)
	);
	return 0;
}

static int meta_isready(lua_State *L) {
	lua_pushboolean(L, World_IsReadyToPlay(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_haserror(lua_State *L) {
	lua_pushboolean(L, World_HasError(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_poperror(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EWorldExtra extra = WORLD_EXTRA_NOINFO;
	EWorldError err = World_PopError(world, &extra);
	lua_pushinteger(L, (lua_Integer)err);
	lua_pushinteger(L, (lua_Integer)extra);
	return 2;
}

static int meta_update(lua_State *L) {
	World_FinishEnvUpdate(
		lua_checkworld(L, 1)
	);
	return 0;
}

static int meta_generate(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	cs_str gname = luaL_checkstring(L, 2);
	cs_int32 seed = (cs_int32)luaL_optinteger(L, 3, GENERATOR_SEED_FROM_TIME);
	GeneratorRoutine func = Generators_Get(gname);
	if(!func) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, "Invalid generator");
	} else {
		World_Lock(world, 0);
		if(!func(world, seed)) {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Generator failed");
		} else {
			lua_pushboolean(L, 1);
			World_Unlock(world);
			return 1;
		}
		World_Unlock(world);
	}

	return 2;
}

static int meta_iterplayers(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	World_Lock(world, 0);
	for(ClientID i = 0; i < MAX_CLIENTS; i++) {
		Client *client = Clients_List[i];
		if(client && Client_IsInWorld(client, world)) {
			lua_pushvalue(L, 2);
			lua_pushclient(L, client);
			if(lua_pcall(L, 1, 0, 0) != 0) {
				World_Unlock(world);
				lua_error(L);
				return 0;
			}
		}
	}
	World_Unlock(world);

	return 0;
}

static int meta_remove(lua_State *L) {
	lua_pushboolean(L, World_Remove(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_unload(lua_State *L) {
	World_Unload(lua_checkworld(L, 1));
	return 0;
}

static int meta_save(lua_State *L) {
	lua_pushboolean(L, World_Save(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_load(lua_State *L) {
	lua_pushboolean(L, World_Load(
		lua_checkworld(L, 1)
	));
	return 1;
}

static int meta_lock(lua_State *L) {
	lua_pushboolean(L, World_Lock(
		lua_checkworld(L, 1),
		(cs_ulong)luaL_optinteger(L, 2, 0)
	));
	return 1;
}

static int meta_unlock(lua_State *L) {
	World_Unlock(lua_checkworld(L, 1));
	return 0;
}

static const luaL_Reg worldmeta[] = {
	{"getname", meta_getname},
	{"getspawn", meta_getspawn},
	{"getoffset", meta_getoffset},
	{"getdimensions", meta_getdimensions},
	{"getblock", meta_getblock},
	{"getenvcolor", meta_getenvcolor},
	{"getenvprop", meta_getenvprop},
	{"getweather", meta_getweather},
	{"gettexpack", meta_gettexpack},
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

static int world_create(lua_State *L) {
	cs_str wname = luaL_checkstring(L, 1);
	SVec *dims = lua_checkshortvector(L, 2);
	luaL_argcheck(L,
		!Vec_IsNegative(*dims) && !Vec_HaveZero(*dims),
		2, "Invalid Vector passed"
	);
	World *world = World_Create(wname);
	World_SetDimensions(world, dims);
	World_AllocBlockArray(world);
	if(!World_IsReadyToPlay(world))
		luaL_error(L, "Failed to create world");
	World_Add(world);
	lua_pushworld(L, world);
	return 1;
}

static int world_getname(lua_State *L) {
	lua_pushworld(L, World_GetByName(
		luaL_checkstring(L, 1)
	));
	return 1;
}

static int world_iterall(lua_State *L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);

	AListField *tmp;
	List_Iter(tmp, World_Head) {
		World *world = AList_GetValue(tmp).ptr;
		lua_pushvalue(L, 1);
		lua_pushworld(L, world);
		if(lua_pcall(L, 1, 0, 0) != 0) {
			lua_error(L);
			return 0;
		}
	}

	return 0;
}

static const luaL_Reg worldlib[] = {
	{"create", world_create},
	{"getbyname", world_getname},
	{"iterall", world_iterall},

	{NULL, NULL}
};

int luaopen_world(lua_State *L) {
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, CSLUA_RWORLDS);
	lua_indexedmeta(L, CSLUA_MWORLD, worldmeta);

	lua_addintconst(L, WORLD_COLOR_SKY);
	lua_addintconst(L, WORLD_COLOR_CLOUD);
	lua_addintconst(L, WORLD_COLOR_FOG);
	lua_addintconst(L, WORLD_COLOR_AMBIENT);
	lua_addintconst(L, WORLD_COLOR_DIFFUSE);

	lua_addintconst(L, WORLD_PROP_SIDEBLOCK);
	lua_addintconst(L, WORLD_PROP_EDGEBLOCK);
	lua_addintconst(L, WORLD_PROP_EDGELEVEL);
	lua_addintconst(L, WORLD_PROP_CLOUDSLEVEL);
	lua_addintconst(L, WORLD_PROP_FOGDIST);
	lua_addintconst(L, WORLD_PROP_SPDCLOUDS);
	lua_addintconst(L, WORLD_PROP_SPDWEATHER);
	lua_addintconst(L, WORLD_PROP_FADEWEATHER);
	lua_addintconst(L, WORLD_PROP_EXPFOG);
	lua_addintconst(L, WORLD_PROP_SIDEOFFSET);

	luaL_newlib(L, worldlib);
	return 1;
}
