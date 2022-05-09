#include <core.h>
#include <str.h>
#include "luascript.h"
#include "luacontact.h"
#include "luaclient.h"
#include "luaworld.h"

static struct _Contact {
	cs_char name[CSLUA_CONTACT_NAMELEN];
	LuaScript *scripts[CSLUA_CONTACT_MAXSCRIPTS];
} contacts[CSLUA_CONTACT_MAX] = {0};

static struct _Contact *newcontact(cs_str name, LuaScript *LS) {
	for(int i = 0; i < CSLUA_CONTACT_MAX; i++) {
		struct _Contact *cont = &contacts[i];
		if(*cont->name == '\0') {
			String_Copy(cont->name, CSLUA_CONTACT_NAMELEN, name);
			cont->scripts[0] = LS;
			return cont;
		}
	}

	return NULL;
}

static struct _Contact *checkcontact(lua_State *L, int idx) {
	struct _Contact *cont = *(struct _Contact **)luaL_checkudata(L, idx, CSLUA_MCONTACT);
	luaL_argcheck(L, *cont->name != '\0', idx, "Contact closed");
	return cont;
}

static struct _Contact *tocontact(lua_State *L, int idx) {
	struct _Contact **cont = (struct _Contact **)luaL_checkudata(L, idx, CSLUA_MCONTACT);
	return cont ? *cont : NULL;
}

static cs_bool addcontactscript(struct _Contact *cont, LuaScript *LS) {
	for(int i = 0; i < CSLUA_CONTACT_MAXSCRIPTS; i++) {
		if(!cont->scripts[i]) {
			cont->scripts[i] = LS;
			return true;
		}
	}

	return false;
}

static void pushcontact(lua_State *L, struct _Contact *cont) {
	if(!cont) {
		lua_pushnil(L);
		return;
	}

	void **ud = lua_newuserdata(L, sizeof(cont));
	luaL_setmetatable(L, CSLUA_MCONTACT);
	*ud = cont;
}

static int meta_pop(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, cont->name);
	lua_rawgeti(L, -1, 0);
	int tablen = (int)lua_objlen(L, -1);
	if(tablen > 0) {
		lua_rawgeti(L, -1, 1);
		for(int pos = 1; pos <= tablen; pos++) {
			lua_rawgeti(L, -2, pos + 1);
			lua_rawseti(L, -3, pos);
		}
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

static void copytable(lua_State *L_to, lua_State *L_from, int idx, cs_str *errt);

static void copyudata(lua_State *L_to, lua_State *L_from, int idx, cs_str *errt) {
	void *tmp = lua_toclient(L_from, idx);
	if(tmp) {
		lua_pushclient(L_to, tmp);
		return;
	}
	tmp = lua_toworld(L_from, idx);
	if(tmp) {
		lua_pushworld(L_to, tmp);
		return;
	}

	if(luaL_getmetafield(L_from, idx, "__name"))
		*errt = lua_tostring(L_from, -1);
	else
		*errt = luaL_typename(L_from, idx);

	return;
}

static int writer(lua_State *L, const void *b, size_t size, void *B) {
	(void)L;
	luaL_addlstring((luaL_Buffer *)B, (const char *)b, size);
	return 0;
}

static const char *reader(lua_State *L, void *b, size_t *size) {
	(void)b;
	return lua_tolstring(L, -1, size);
}

static void copyfunction(lua_State *L_to, lua_State *L_from, int idx, cs_str *errt) {
	if(lua_iscfunction(L_from, idx)) {
		*errt = "C function";
		return;
	} else
		*errt = lua_typename(L_from, idx);

	luaL_Buffer b;
	luaL_buffinit(L_to, &b);
	lua_pushvalue(L_from, idx);
#	if LUA_VERSION_NUM > 502
		if(lua_dump(L_from, writer, &b, 0) != 0)
			return;
		luaL_pushresult(&b);
#	else
		if(lua_dump(L_from, writer, &b) != 0)
			return;
		luaL_pushresult(&b);
#	endif

#	if LUA_VERSION_NUM > 501
		if(lua_load(L_to, reader, NULL, "transfered chunk", "binary"))
			return;
#	else
		if(lua_load(L_to, reader, NULL, "transfered chunk"))
			return;
#	endif

	lua_remove(L_to, -2);
	*errt = NULL;
}

static cs_bool copyvalue(lua_State *L_to, lua_State *L_from, int idx, cs_str *errt) {
	switch(lua_type(L_from, idx)) {
		case LUA_TNIL:
			lua_pushnil(L_to);
			break;

		case LUA_TBOOLEAN:
			lua_pushboolean(L_to, lua_toboolean(L_from, idx));
			break;

		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L_to, lua_touserdata(L_from, idx));
			break;

		case LUA_TNUMBER:
			lua_pushnumber(L_to, lua_tonumber(L_from, idx));
			break;

		case LUA_TSTRING:
			lua_pushstring(L_to, lua_tostring(L_from, idx));
			break;

		case LUA_TTABLE:
			copytable(L_to, L_from, idx, errt);
			break;

		case LUA_TUSERDATA:
			copyudata(L_to, L_from, idx, errt);
			break;

		case LUA_TFUNCTION:
			copyfunction(L_to, L_from, idx, errt);
			break;

		default:
			*errt = luaL_typename(L_from, idx);
	}

	return *errt == NULL;
}

static void copytable(lua_State *L_to, lua_State *L_from, int idx, cs_str *errt) {
	// Превращаем относительный индекс в абсолютный
	if(idx < 0) idx = lua_gettop(L_from) + idx + 1;

	lua_pushnil(L_from);
	lua_newtable(L_to);
	while(lua_next(L_from, idx) != 0) {
		if(lua_rawequal(L_from, idx, -2)) // Если ключ сама же эта таблица, то её и пушим
			lua_pushvalue(L_to, -1);
		else if(!copyvalue(L_to, L_from, -2, errt))
			return;

		if(lua_rawequal(L_from, idx, -1))
			lua_pushvalue(L_to, -2);
		else if(!copyvalue(L_to, L_from, -1, errt))
			return;

		lua_rawset(L_to, -3);
		lua_pop(L_from, 1);
	}

	return;
}

static int meta_push(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);
	cs_str errt = NULL;

	for(int i = 0; i < CSLUA_CONTACT_MAXSCRIPTS; i++) {
		LuaScript *_LS = cont->scripts[i];
		if(!_LS) continue;

		if(_LS->L != L) {
			LuaScript_Lock(_LS);
			int tstart = lua_gettop(_LS->L);
			lua_getfield(_LS->L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
			lua_getfield(_LS->L, -1, cont->name);
			lua_rawgeti(_LS->L, -1, 0);
			if(!copyvalue(_LS->L, L, 2, &errt)) {
				lua_settop(_LS->L, tstart);
				LuaScript_Unlock(_LS);
				luaL_error(L, "Attempt to push an unsupported value: %s", errt);
				return 0;
			}
			lua_rawseti(_LS->L, -2, (int)lua_objlen(_LS->L, -2) + 1);
			lua_rawgeti(_LS->L, -2, 2);
			if(lua_isfunction(_LS->L, -1)) {
				lua_rawgeti(_LS->L, -3, 1);
				(void)LuaScript_Call(_LS, 1, 0);
			}
			lua_settop(_LS->L, tstart);
			LuaScript_Unlock(_LS);
		}
	}

	return 0;
}

static int meta_bind(lua_State *L) {
	luaL_checktype(L, 2, LUA_TFUNCTION);
	struct _Contact *cont = checkcontact(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, cont->name);
	lua_pushvalue(L, 2);
	lua_rawseti(L, -2, 2); // cscontact[name][2] = <2>
	return 0;
}

static int meta_avail(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, cont->name);
	lua_rawgeti(L, -1, 0);
	lua_pushinteger(L, lua_objlen(L, -1));
	return 1;
}

static int meta_clear(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, cont->name);
	lua_rawgeti(L, -1, 0);
	for(int i = (int)lua_objlen(L, -1); i > 0; i--) {
		lua_pushnil(L);
		lua_rawseti(L, -2, i);
	}
	return 0;
}

static int meta_close(lua_State *L) {
	struct _Contact *cont = tocontact(L, 1);
	if(!cont || *cont->name == '\0') return 0;
	cs_bool dirty = false;

	for(int i = 0; i < CSLUA_CONTACT_MAXSCRIPTS; i++) {
		LuaScript *_LS = cont->scripts[i];
		if(_LS) {
			if(_LS->L == L) {
				cont->scripts[i] = NULL;
				continue;
			}
			dirty = true;
		}
	}

	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_pushnil(L);
	lua_setfield(L, -2, cont->name);

	if(!dirty)
		*cont->name = '\0';

	return 0;
}

const luaL_Reg contactmeta[] = {
	{"pop", meta_pop},
	{"push", meta_push},
	{"bind", meta_bind},

	{"avail", meta_avail},
	{"clear", meta_clear},
	{"close", meta_close},

	{"__gc", meta_close},

	{NULL, NULL}
};

static int cont_get(lua_State *L) {
	cs_size namelen = 0;
	cs_str name = luaL_checklstring(L, 1, &namelen);
	luaL_argcheck(L, namelen < CSLUA_CONTACT_NAMELEN, 1, "Too long contact name");
	LuaScript *LS = lua_getscript(L);

	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, name); // cscontact[name]
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		for(int i = 0; i < CSLUA_CONTACT_MAX; i++) {
			struct _Contact *cont = &contacts[i];
			if(String_Compare(cont->name, name)) {
				if(addcontactscript(cont, LS)) {
					pushcontact(L, cont);
					goto cfinish;
				} else
					luaL_error(L, "Failed to connect to specified contact");
				return 1;
			}
		}

		pushcontact(L, newcontact(name, LS));

		cfinish:
		lua_newtable(L);
		lua_newtable(L);
		lua_rawseti(L, -2, 0);
		lua_pushvalue(L, -2); // Contact
		lua_rawseti(L, -2, 1);
		lua_setfield(L, -3, name);
		return 1;
	}

	lua_rawgeti(L, -1, 1);
	return 1;
}

const luaL_Reg contactlib[] = {
	{"get", cont_get},

	{NULL, NULL}
};

int luaopen_contact(lua_State *L) {
	lua_createtable(L, 0, CSLUA_CONTACT_MAX);
	lua_setfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);

	lua_indexedmeta(L, CSLUA_MCONTACT, contactmeta);
	luaL_newlib(L, contactlib);
	return 1;
}
