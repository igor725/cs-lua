#include <core.h>
#include <config.h>
#include "luaplugin.h"
#include "luaconfig.h"

CStore *lua_checkcfgstore(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, "Config");
	luaL_argcheck(L, *ud != NULL, idx, "Invalid config");
	return (CStore *)*ud;
}

const luaL_Reg configmeta[] = {
	{NULL, NULL}
};

static int config_new(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, -1, "name");
	cs_str cname = luaL_checkstring(L, -1);
	lua_getfield(L, -2, "items");
	luaL_checktype(L, -1, LUA_TTABLE);
	CStore *store = Config_NewStore(cname);
	size_t itemcnt = lua_objlen(L, -1);
	if(itemcnt < 1) {
		Config_DestroyStore(store);
		return 0;
	}

	for(size_t i = 1; i < itemcnt; i++) {
		lua_rawgeti(L, -1, (int)i);
		lua_getfield(L, -1, "name");
		lua_getfield(L, -2, "type");
		lua_getfield(L, -3, "comment");
		lua_getfield(L, -4, "default");
		ECTypes type = lua_tointeger(L, -3);
		if(!lua_isstring(L, -4) || !lua_isnumber(L, -3)
		|| type >= CONFIG_MAX_TYPE) {
			lua_pop(L, 7);
			Config_DestroyStore(store);
			luaL_error(L, "Invalid store entry #%d", i);
			return 0;
		}
		cs_str key = lua_tostring(L, -4);
		CEntry *ent = Config_NewEntry(store, key, type);
		if(lua_isstring(L, -2))
			Config_SetComment(ent, lua_tostring(L, -2));
		lua_pop(L, 5);
	}
	lua_pop(L, 2);
	CStore **ud = lua_newuserdata(L, sizeof(CStore *));
	luaL_setmetatable(L, "Config");
	*ud = store;
	return 1;
}

const luaL_Reg configlib[] = {
	{"new", config_new},
	{NULL, NULL}
};

int luaopen_config(lua_State *L) {
	luaL_newmetatable(L, "Config");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, configmeta, 0);
	lua_pop(L, 1);

	lua_addnumconst(L, CONFIG_ERROR_SUCCESS);
	lua_addnumconst(L, CONFIG_ERROR_INTERNAL);
	lua_addnumconst(L, CONFIG_ERROR_IOFAIL);
	lua_addnumconst(L, CONFIG_ERROR_PARSE);

	lua_addnumconst(L, CONFIG_EXTRA_NOINFO);
	lua_addnumconst(L, CONFIG_EXTRA_IO_LINEASERROR);
	lua_addnumconst(L, CONFIG_EXTRA_IO_FRENAME);
	lua_addnumconst(L, CONFIG_EXTRA_PARSE_NOENTRY);
	lua_addnumconst(L, CONFIG_EXTRA_PARSE_LINEFORMAT);
	lua_addnumconst(L, CONFIG_EXTRA_PARSE_NUMBER);
	lua_addnumconst(L, CONFIG_EXTRA_PARSE_END);

	lua_addnumconst(L, CONFIG_TYPE_BOOL);
	lua_addnumconst(L, CONFIG_TYPE_INT32);
	lua_addnumconst(L, CONFIG_TYPE_INT16);
	lua_addnumconst(L, CONFIG_TYPE_INT8);
	lua_addnumconst(L, CONFIG_TYPE_STR);

	luaL_register(L, luaL_checkstring(L, 1), configlib);
	return 1;
}
