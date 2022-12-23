#include <core.h>
#include <config.h>
#include "luascript.h"
#include "luaconfig.h"

CStore *scr_checkcfgstore(scr_Context *L, int idx) {
	void **ud = scr_checkmemtype(L, idx, CSSCRIPTS_MCONFIG);
	scr_argassert(L, *ud != NULL, idx, "Invalid config");
	return (CStore *)*ud;
}

static int meta_get(scr_Context *L) {
	CEntry *ent = Config_GetEntry(
		scr_checkcfgstore(L, 1),
		scr_checkstring(L, 2)
	);
	scr_argassert(L, ent != NULL, 2, "Config entry not found");
	switch (ent->type) {
		case CONFIG_TYPE_BOOL:
			scr_pushboolean(L, (cs_bool)Config_GetBool(ent));
			break;
		case CONFIG_TYPE_INT:
			scr_pushinteger(L, (scr_Integer)Config_GetInt(ent));
			break;
		case CONFIG_TYPE_STR:
			scr_pushstring(L, Config_GetStr(ent));
			break;

		case CONFIG_MAX_TYPE:
		default:
			scr_pushstring(L, "Internal error");
			scr_error(L);
			return 0;
	}

	return 1;
}

static int meta_set(scr_Context *L) {
	CEntry *ent = Config_GetEntry(
		scr_checkcfgstore(L, 1),
		scr_checkstring(L, 2)
	);
	scr_argassert(L, ent != NULL, 2, "Config entry not found");
	switch (ent->type) {
		case CONFIG_TYPE_BOOL:
			Config_SetBool(ent, scr_toboolean(L, 3));
			break;
		case CONFIG_TYPE_INT:
			Config_SetInt(ent, (cs_int32)scr_tointeger(L, 3));
			break;
		case CONFIG_TYPE_STR:
			Config_SetStr(ent, scr_checkstring(L, 3));
			break;

		case CONFIG_MAX_TYPE:
		default:
			scr_pushstring(L, "Internal error");
			scr_error(L);
			break;
	}

	return 0;
}

static int meta_load(scr_Context *L) {
	scr_pushboolean(L, Config_Load(
		scr_checkcfgstore(L, 1)
	));
	return 1;
}

static int meta_save(scr_Context *L) {
	scr_pushboolean(L, Config_Save(
		scr_checkcfgstore(L, 1),
		scr_toboolean(L, 2)
	));
	return 1;
}

static int meta_reset(scr_Context *L) {
	Config_ResetToDefault(
		scr_checkcfgstore(L, 1)
	);
	return 0;
}

static int meta_poperror(scr_Context *L) {
	ECExtra extra = CONFIG_EXTRA_NOINFO;
	cs_int32 line = 0;
	ECError error = Config_PopError(
		scr_checkcfgstore(L, 1),
		&extra, &line
	);
	scr_pushinteger(L, (scr_Integer)error);
	scr_pushinteger(L, (scr_Integer)extra);
	scr_pushinteger(L, (scr_Integer)line);
	return 3;
}

static int meta_destroy(scr_Context *L) {
	CStore *store = scr_checkcfgstore(L, 1);
	*(void **)scr_getmem(L, 1) = NULL;
	Config_DestroyStore(store);
	return 0;
}

const scr_RegFuncs configmeta[] = {
	{"get", meta_get},
	{"set", meta_set},

	{"load", meta_load},
	{"save", meta_save},
	{"reset", meta_reset},
	{"poperror", meta_poperror},

	{"__gc", meta_destroy},

	{NULL, NULL}
};

static int config_new(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	scr_gettabfield(L, -1, "name");
	cs_str cname = scr_checkstring(L, -1);
	scr_gettabfield(L, -2, "items");
	luaL_checktype(L, -1, LUA_TTABLE);
	CStore *store = Config_NewStore(cname);
	size_t itemcnt = scr_gettablen(L, -1);
	if(itemcnt < 1) {
		Config_DestroyStore(store);
		return 0;
	}

	scr_stackcheck(L, 5);
	for(size_t i = 1; i <= itemcnt; i++) {
		scr_rawgeti(L, -1, (int)i);
		scr_gettabfield(L, -1, "name");
		scr_gettabfield(L, -2, "type");
		scr_gettabfield(L, -3, "comment");
		scr_gettabfield(L, -4, "default");
		ECTypes type = scr_tointeger(L, -3);
		if(!scr_isstring(L, -4) || !scr_isnumber(L, -3)
		|| type >= CONFIG_MAX_TYPE) {
			Config_DestroyStore(store);
			scr_fmterror(L, "Invalid store entry #%d", i);
			return 0;
		}
		cs_str key = scr_tostring(L, -4);
		CEntry *ent = Config_NewEntry(store, key, type);
		if(scr_isstring(L, -2))
			Config_SetComment(ent, scr_tostring(L, -2));
		if(!scr_isnull(L, -1)) {
			switch(type) {
				case CONFIG_TYPE_BOOL:
					Config_SetDefaultBool(ent, scr_toboolean(L, -1));
					break;
				case CONFIG_TYPE_INT:
					Config_SetDefaultInt(ent, (cs_int32)scr_tointeger(L, -1));
					break;
				case CONFIG_TYPE_STR:
					Config_SetDefaultStr(ent,
						scr_isstring(L, -1) ? scr_tostring(L, -1) : ""
					);
					break;

				case CONFIG_MAX_TYPE:
				default:
					Config_DestroyStore(store);
					scr_pushstring(L, "Internal error");
					scr_error(L);
					break;
			}
		}
		scr_stackpop(L, 5);
	}
	scr_stackpop(L, 2);
	CStore **ud = scr_allocmem(L, sizeof(CStore *));
	scr_setmemtype(L, CSSCRIPTS_MCONFIG);
	*ud = store;
	return 1;
}

static int config_error(scr_Context *L) {
	scr_pushstring(L, Config_ErrorToString(
		(ECError)scr_checkinteger(L, 1)
	));
	scr_pushstring(L, Config_ExtraToString(
		(ECExtra)scr_checkinteger(L, 2)
	));
	return 2;
}

const scr_RegFuncs configlib[] = {
	{"new", config_new},
	{"error", config_error},

	{NULL, NULL}
};

int scr_libfunc(config)(scr_Context *L) {
	scr_createtype(L, CSSCRIPTS_MCONFIG, configmeta);

	scr_addintconst(L, CONFIG_ERROR_SUCCESS);
	scr_addintconst(L, CONFIG_ERROR_INTERNAL);
	scr_addintconst(L, CONFIG_ERROR_IOFAIL);
	scr_addintconst(L, CONFIG_ERROR_PARSE);

	scr_addintconst(L, CONFIG_EXTRA_NOINFO);
	scr_addintconst(L, CONFIG_EXTRA_IO_LINEASERROR);
	scr_addintconst(L, CONFIG_EXTRA_IO_FRENAME);
	scr_addintconst(L, CONFIG_EXTRA_PARSE_LINEFORMAT);
	scr_addintconst(L, CONFIG_EXTRA_PARSE_NUMBER);
	scr_addintconst(L, CONFIG_EXTRA_PARSE_END);

	scr_addintconst(L, CONFIG_TYPE_BOOL);
	scr_addintconst(L, CONFIG_TYPE_INT);
	scr_addintconst(L, CONFIG_TYPE_STR);

	scr_newlib(L, configlib);
	return 1;
}
