#include <core.h>
#include <config.h>
#include "luascript.h"
#include "luaconfig.h"

CStore *lua_checkcfgstore(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, CSLUA_MCONFIG);
	luaL_argcheck(L, *ud != NULL, idx, "Invalid config");
	return (CStore *)*ud;
}

static int meta_get(lua_State *L) {
	CEntry *ent = Config_GetEntry(
		lua_checkcfgstore(L, 1),
		luaL_checkstring(L, 2)
	);
	luaL_argcheck(L, ent != NULL, 2, "Config entry not found");
	switch (ent->type) {
		case CONFIG_TYPE_BOOL:
			lua_pushboolean(L, (cs_bool)Config_GetBool(ent));
			break;
		case CONFIG_TYPE_INT:
			lua_pushinteger(L, (lua_Integer)Config_GetInt(ent));
			break;
		case CONFIG_TYPE_STR:
			lua_pushstring(L, Config_GetStr(ent));
			break;

		case CONFIG_MAX_TYPE:
		default:
			lua_pushstring(L, "Internal error");
			lua_error(L);
			return 0;
	}

	return 1;
}

static int meta_set(lua_State *L) {
	CEntry *ent = Config_GetEntry(
		lua_checkcfgstore(L, 1),
		luaL_checkstring(L, 2)
	);
	luaL_argcheck(L, ent != NULL, 2, "Config entry not found");
	switch (ent->type) {
		case CONFIG_TYPE_BOOL:
			Config_SetBool(ent, (cs_bool)lua_toboolean(L, 3));
			break;
		case CONFIG_TYPE_INT:
			Config_SetInt(ent, (cs_int32)lua_tointeger(L, 3));
			break;
		case CONFIG_TYPE_STR:
			Config_SetStr(ent, luaL_checkstring(L, 3));
			break;

		case CONFIG_MAX_TYPE:
		default:
			lua_pushstring(L, "Internal error");
			lua_error(L);
			break;
	}

	return 0;
}

static int meta_load(lua_State *L) {
	lua_pushboolean(L, Config_Load(
		lua_checkcfgstore(L, 1)
	));
	return 1;
}

static int meta_save(lua_State *L) {
	lua_pushboolean(L, Config_Save(
		lua_checkcfgstore(L, 1),
		(cs_bool)lua_toboolean(L, 2)
	));
	return 1;
}

static int meta_reset(lua_State *L) {
	Config_ResetToDefault(
		lua_checkcfgstore(L, 1)
	);
	return 0;
}

static int meta_poperror(lua_State *L) {
	ECExtra extra = CONFIG_EXTRA_NOINFO;
	cs_int32 line = 0;
	ECError error = Config_PopError(
		lua_checkcfgstore(L, 1),
		&extra, &line
	);
	lua_pushinteger(L, (lua_Integer)error);
	lua_pushinteger(L, (lua_Integer)extra);
	lua_pushinteger(L, (lua_Integer)line);
	return 3;
}

static int meta_destroy(lua_State *L) {
	CStore *store = lua_checkcfgstore(L, 1);
	*(void **)lua_touserdata(L, 1) = NULL;
	Config_DestroyStore(store);
	return 0;
}

const luaL_Reg configmeta[] = {
	{"get", meta_get},
	{"set", meta_set},

	{"load", meta_load},
	{"save", meta_save},
	{"reset", meta_reset},
	{"poperror", meta_poperror},

	{"__gc", meta_destroy},

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

	lua_checkstack(L, 5);
	for(size_t i = 1; i <= itemcnt; i++) {
		lua_rawgeti(L, -1, (int)i);
		lua_getfield(L, -1, "name");
		lua_getfield(L, -2, "type");
		lua_getfield(L, -3, "comment");
		lua_getfield(L, -4, "default");
		ECTypes type = lua_tointeger(L, -3);
		if(!lua_isstring(L, -4) || !lua_isnumber(L, -3)
		|| type >= CONFIG_MAX_TYPE) {
			Config_DestroyStore(store);
			luaL_error(L, "Invalid store entry #%d", i);
			return 0;
		}
		cs_str key = lua_tostring(L, -4);
		CEntry *ent = Config_NewEntry(store, key, type);
		if(lua_isstring(L, -2))
			Config_SetComment(ent, lua_tostring(L, -2));
		if(!lua_isnil(L, -1)) {
			switch(type) {
				case CONFIG_TYPE_BOOL:
					Config_SetDefaultBool(ent, (cs_bool)lua_toboolean(L, -1));
					break;
				case CONFIG_TYPE_INT:
					Config_SetDefaultInt(ent, (cs_int32)lua_tointeger(L, -1));
					break;
				case CONFIG_TYPE_STR:
					Config_SetDefaultStr(ent,
						lua_isstring(L, -1) ? lua_tostring(L, -1) : ""
					);
					break;

				case CONFIG_MAX_TYPE:
				default:
					Config_DestroyStore(store);
					lua_pushstring(L, "Internal error");
					lua_error(L);
					break;
			}
		}
		lua_pop(L, 5);
	}
	lua_pop(L, 2);
	CStore **ud = lua_newuserdata(L, sizeof(CStore *));
	luaL_setmetatable(L, CSLUA_MCONFIG);
	*ud = store;
	return 1;
}

static int config_error(lua_State *L) {
	lua_pushstring(L, Config_ErrorToString(
		(ECError)luaL_checkinteger(L, 1)
	));
	lua_pushstring(L, Config_ExtraToString(
		(ECExtra)luaL_checkinteger(L, 2)
	));
	return 2;
}

const luaL_Reg configlib[] = {
	{"new", config_new},
	{"error", config_error},

	{NULL, NULL}
};

int luaopen_config(lua_State *L) {
	lua_indexedmeta(L, CSLUA_MCONFIG, configmeta);

	lua_addintconst(L, CONFIG_ERROR_SUCCESS);
	lua_addintconst(L, CONFIG_ERROR_INTERNAL);
	lua_addintconst(L, CONFIG_ERROR_IOFAIL);
	lua_addintconst(L, CONFIG_ERROR_PARSE);

	lua_addintconst(L, CONFIG_EXTRA_NOINFO);
	lua_addintconst(L, CONFIG_EXTRA_IO_LINEASERROR);
	lua_addintconst(L, CONFIG_EXTRA_IO_FRENAME);
	lua_addintconst(L, CONFIG_EXTRA_PARSE_LINEFORMAT);
	lua_addintconst(L, CONFIG_EXTRA_PARSE_NUMBER);
	lua_addintconst(L, CONFIG_EXTRA_PARSE_END);

	lua_addintconst(L, CONFIG_TYPE_BOOL);
	lua_addintconst(L, CONFIG_TYPE_INT);
	lua_addintconst(L, CONFIG_TYPE_STR);

	luaL_newlib(L, configlib);
	return 1;
}
