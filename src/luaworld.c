#include <core.h>
#include <str.h>
#include <world.h>
#include "luaplugin.h"
#include "luaworld.h"
#include "luavector.h"
#include "luaangle.h"

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
	LuaVector *vec = lua_newluavector(L);
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
	EWorldColors ctype = (EWorldColors)luaL_checkinteger(L, 2);
	Color3 *color = World_GetEnvColor(world, ctype);
	if(color) {
		lua_pushinteger(L, color->r);
		lua_pushinteger(L, color->g);
		lua_pushinteger(L, color->b);
		return 3;
	}

	return 0;
}

static int meta_getenvprop(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EWorldProp ptype = (EWorldProp)luaL_checkinteger(L, 2);
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

static int meta_setenvcolor(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EWorldColors ctype = (EWorldColors)luaL_checkinteger(L, 2);
	Color3 color = {
		.r = (cs_int16)luaL_checkinteger(L, 3),
		.g = (cs_int16)luaL_checkinteger(L, 4),
		.b = (cs_int16)luaL_checkinteger(L, 5)
	};
	lua_pushboolean(L, World_SetEnvColor(world, ctype, &color));
	return 1;
}

static int meta_setenvprop(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EWorldProp ptype = (EWorldProp)luaL_checkinteger(L, 2);
	cs_int32 pvalue = (cs_int32)luaL_checkinteger(L, 3);
	lua_pushboolean(L, World_SetEnvProp(world, ptype, pvalue));
	return 1;
}

static int meta_setweather(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	EWorldWeather wtype = (cs_int32)luaL_checkinteger(L, 2);
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

static int meta_pushenvupdates(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	World_FinishEnvUpdate(world);
	return 0;
}

static const luaL_Reg worldmeta[] = {
	{"getname", meta_getname},
	{"getdimensions", meta_getdimensions},
	{"getdimensionsa", meta_getdimensionsa},
	{"getblock", meta_getblock},
	{"getenvcolor", meta_getenvcolor},
	{"getenvprop", meta_getenvprop},
	{"getweather", meta_getweather},
	{"gettexpack", meta_gettexpack},

	{"setblock", meta_setblock},
	{"setenvcolor", meta_setenvcolor},
	{"setenvprop", meta_setenvprop},
	{"setweather", meta_setweather},
	{"settexpack", meta_settexpack},

	{"pushenvupdates", meta_pushenvupdates},

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

	luaL_register(L, "world", worldlib);
	return 1;
}
