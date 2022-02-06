#include <core.h>
#include <world.h>
#include "luaplugin.h"
#include "luaworld.h"
#include "luavector.h"
#include "luaangle.h"

World *lua_checkworld(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, "World");
	luaL_argcheck(L, ud != NULL, idx, "'World' expected");
	return (World *)*ud;
}

void lua_pushworld(lua_State *L, World *world) {
	if(!world) {
		lua_pushnil(L);
		return;
	}

	cs_str name = World_GetName(world);
	lua_pushstring(L, "__worlds");
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushstring(L, name);
		lua_newuserdata(L, sizeof(World *));
		luaL_setmetatable(L, "World");
		lua_settable(L, -3);
		lua_pushstring(L, name);
		lua_gettable(L, -2);
	}

	lua_remove(L, -2);
	void **ud = lua_touserdata(L, -1);
	*ud = world;
}

static int world_getname(lua_State *L) {
	cs_str name = (cs_str)luaL_checkstring(L, 1);
	lua_pushworld(L, World_GetByName(name));
	return 1;
}

static const luaL_Reg worldlib[] = {
	{"getbyname", world_getname},
	{NULL, NULL}
};

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

static int meta_getblock(lua_State *L) {
	World *world = lua_checkworld(L, 1);
	LuaVector *vec = lua_checkvector(L, 2);
	if(vec->type == 1)
		lua_pushinteger(L, World_GetBlock(world, &vec->value.s));
	else lua_pushinteger(L, 0);
	return 1;
}

static const luaL_Reg worldmeta[] = {
	{"getname", meta_getname},
	{"getdimensions", meta_getdimensions},
	{"getblock", meta_getblock},
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

	luaL_register(L, "world", worldlib);
	return 1;
}
