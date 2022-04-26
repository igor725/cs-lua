#include <core.h>
#include <groups.h>
#include "luascript.h"
#include "luagroups.h"

static int group_create(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)Groups_Create(
		luaL_checkstring(L, 1),
		(cs_byte)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int group_remove(lua_State *L) {
	lua_pushboolean(L, Groups_Remove(
		(cs_uintptr)luaL_checkinteger(L, 1)
	));
	return 1;
}

static int group_getinfo(lua_State *L) {
	CGroup *grp = Groups_GetByID((cs_uintptr)luaL_checkinteger(L, 1));
	if(grp) {
		lua_pushinteger(L, (lua_Integer)grp->rank);
		lua_pushstring(L, grp->name);
		return 2;
	}

	lua_pushnil(L);
	return 1;
}

static const luaL_Reg groupslib[] = {
	{"create", group_create},
	{"remove", group_remove},
	{"getinfo", group_getinfo},

	{NULL, NULL}
};

int luaopen_groups(lua_State *L) {
	lua_addintconst(L, GROUPS_INVALID_ID);

	luaL_newlib(L, groupslib);
	return 1;
}
