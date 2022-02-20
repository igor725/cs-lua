#include <core.h>
#include <str.h>
#include <world.h>
#include <client.h>
#include "luaplugin.h"
#include "luaworld.h"
#include "luavector.h"
#include "luaangle.h"
#include "luacolor.h"
#include "luaclient.h"

World *lua_checkworld(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, "World");
	return (World *)*ud;
}

void lua_pushworld(lua_State *L, World *world) {
	if(!world) {
		lua_pushnil(L);
		return;
	}

	cs_str name = World_GetName(world);
	lua_getfield(L, LUA_REGISTRYINDEX, "__worlds");
	lua_getfield(L, -1, name);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_newuserdata(L, sizeof(World *));
		luaL_setmetatable(L, "World");
		lua_setfield(L, -2, name);
		lua_getfield(L, -1, name);
	}

	lua_remove(L, -2);
	void **ud = lua_touserdata(L, -1);
	*ud = world;
}

static int meta_getname(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushstring(L, World_GetName(world));
	return 1;
}

static int meta_getdimensions(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	LuaVector *vec = lua_checkvector(L, 2);
	if(vec->type == 1) {
		World_GetDimensions(world, &vec->value.s);
		lua_pushboolean(L, true);
	} else lua_pushboolean(L, false);
	return 1;
}

static int meta_getdimensionsa(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	LuaVector *vec = lua_newvector(L);
	World_GetDimensions(world, &vec->value.s);
	vec->type = 1;
	return 1;
}

static int meta_getblock(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	LuaVector *vec = lua_checkvector(L, 2);
	if(vec->type == 1)
		lua_pushinteger(L, World_GetBlock(world, &vec->value.s));
	else lua_pushinteger(L, 0);
	return 1;
}

static int meta_getenvcolor(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EColors ctype = (EColors)luaL_checkinteger(L, 2);
	Color3 *col = lua_checkcolor3(L, 3);
	*col = *World_GetEnvColor(world, ctype);
	return 1;
}

static int meta_getenvcolora(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EColors ctype = (EColors)luaL_checkinteger(L, 2);
	LuaColor *col = lua_newcolor(L);
	col->value.c3 = *World_GetEnvColor(world, ctype);
	col->hasAlpha = false;
	return 1;
}

static int meta_getenvprop(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EProp ptype = (EProp)luaL_checkinteger(L, 2);
	lua_pushinteger(L, (lua_Integer)World_GetEnvProp(world, ptype));
	return 1;
}

static int meta_getweather(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushinteger(L, (lua_Integer)World_GetWeather(world));
	return 1;
}

static int meta_gettexpack(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushstring(L, World_GetTexturePack(world));
	return 1;
}

static int meta_setblock(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	SVec *pos = lua_checkshortvector(L, 2);
	BlockID id = (BlockID)luaL_checkinteger(L, 3);
	lua_pushboolean(L, World_SetBlock(world, pos, id));
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
	World *world = lua_checkworld(L, 1);
	EColors ctype = (EColors)luaL_checkinteger(L, 2);
	Color3 *col = lua_checkcolor3(L, 3);
	lua_pushboolean(L, World_SetEnvColor(world, ctype, col));
	return 1;
}

static int meta_setenvprop(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EProp ptype = (EProp)luaL_checkinteger(L, 2);
	cs_int32 pvalue = (cs_int32)luaL_checkinteger(L, 3);
	lua_pushboolean(L, World_SetEnvProp(world, ptype, pvalue));
	return 1;
}

static int meta_setweather(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EWeather wtype = (cs_int32)luaL_checkinteger(L, 2);
	lua_pushboolean(L, World_SetWeather(world, wtype));
	return 1;
}

static int meta_settexpack(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	cs_str texpack = (cs_str)luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(texpack) < 64, 2, "URL too long");
	lua_pushboolean(L, World_SetTexturePack(world, texpack));
	return 1;
}

static int meta_isready(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushboolean(L, World_IsReadyToPlay(world));
	return 1;
}

static int meta_haserror(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushboolean(L, World_HasError(world));
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
	World *world = lua_checkworld(L, 1);
	World_FinishEnvUpdate(world);
	return 0;
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

static int meta_unload(lua_State *L) {
	World_Unload(lua_checkworld(L, 1));
	return 0;
}

static int meta_save(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushboolean(L, World_Save(world));
	return 1;
}

static int meta_load(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	lua_pushboolean(L, World_Load(world));
	return 1;
}

static int meta_lock(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	cs_ulong timeout = (cs_ulong)luaL_optinteger(L, 2, 0);
	lua_pushboolean(L, World_Lock(world, timeout));
	return 1;
}

static int meta_unlock(lua_State *L) {
	World_Unlock(lua_checkworld(L, 1));
	return 0;
}

static const luaL_Reg worldmeta[] = {
	{"getname", meta_getname},
	{"getdimensions", meta_getdimensions},
	{"getdimensionsa", meta_getdimensionsa},
	{"getblock", meta_getblock},
	{"getenvcolor", meta_getenvcolor},
	{"getenvcolora", meta_getenvcolora},
	{"getenvprop", meta_getenvprop},
	{"getweather", meta_getweather},
	{"gettexpack", meta_gettexpack},

	{"setblock", meta_setblock},
	{"setblocknat", meta_setblocknat},
	{"setenvcolor", meta_setenvcolor},
	{"setenvprop", meta_setenvprop},
	{"setweather", meta_setweather},
	{"settexpack", meta_settexpack},

	{"isready", meta_isready},

	{"haserror", meta_haserror},
	{"poperror", meta_poperror},

	{"lock", meta_lock},
	{"unlock", meta_unlock},

	{"update", meta_update},
	{"iterplayers", meta_iterplayers},

	{"unload", meta_unload},
	{"save", meta_save},
	{"load", meta_load},

	{NULL, NULL}
};

static int world_getname(lua_State *L) {
	cs_str name = (cs_str)luaL_checkstring(L, 1);
	lua_pushworld(L, World_GetByName(name));
	return 1;
}

static const luaL_Reg worldlib[] = {
	{"getbyname", world_getname},
	{NULL, NULL}
};

int luaopen_world(lua_State *L) {
	lua_pushstring(L, "__worlds");
	lua_newtable(L);
	lua_settable(L, LUA_REGISTRYINDEX);

	luaL_newmetatable(L, "World");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, worldmeta, 0);
	lua_pop(L, 1);

	lua_addnumconst(L, WORLD_COLOR_SKY);
	lua_addnumconst(L, WORLD_COLOR_CLOUD);
	lua_addnumconst(L, WORLD_COLOR_FOG);
	lua_addnumconst(L, WORLD_COLOR_AMBIENT);
	lua_addnumconst(L, WORLD_COLOR_DIFFUSE);

	lua_addnumconst(L, WORLD_PROP_SIDEBLOCK);
	lua_addnumconst(L, WORLD_PROP_EDGEBLOCK);
	lua_addnumconst(L, WORLD_PROP_EDGELEVEL);
	lua_addnumconst(L, WORLD_PROP_CLOUDSLEVEL);
	lua_addnumconst(L, WORLD_PROP_FOGDIST);
	lua_addnumconst(L, WORLD_PROP_SPDCLOUDS);
	lua_addnumconst(L, WORLD_PROP_SPDWEATHER);
	lua_addnumconst(L, WORLD_PROP_FADEWEATHER);
	lua_addnumconst(L, WORLD_PROP_EXPFOG);
	lua_addnumconst(L, WORLD_PROP_SIDEOFFSET);

	luaL_register(L, luaL_checkstring(L, 1), worldlib);
	return 1;
}
