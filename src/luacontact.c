#include <core.h>
#include <list.h>
#include <str.h>
#include <log.h>
#include <platform.h>
#include "luascript.h"
#include "luacontact.h"

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
		lua_setfield(L, -2, "queue");
		lua_setfield(L, -3, name);
		return 1;
	}

	return 1;
}

const luaL_Reg contactlib[] = {
	{"get", cont_get},

	{NULL, NULL}
};

static int meta_pop(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, cont->name);
	lua_getfield(L, -1, "queue");
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

static int meta_push(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);

	for(int i = 0; i < CSLUA_CONTACT_MAXSCRIPTS; i++) {
		LuaScript *_LS = cont->scripts[i];
		if(!_LS) continue;

		if(_LS->L != L) {
			LuaScript_Lock(_LS);
			lua_getfield(_LS->L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
			lua_getfield(_LS->L, -1, cont->name);
			lua_getfield(_LS->L, -1, "queue");

			switch(lua_type(L, 2)) {
				case LUA_TNIL:
					lua_pushnil(_LS->L);
					break;

				case LUA_TBOOLEAN:
					lua_pushboolean(_LS->L, lua_toboolean(L, 2));
					break;

				case LUA_TLIGHTUSERDATA:
					lua_pushlightuserdata(_LS->L, lua_touserdata(L, 2));
					break;

				case LUA_TNUMBER:
					lua_pushstring(_LS->L, lua_tostring(L, 2));
					break;

				case LUA_TSTRING:
					lua_pushstring(_LS->L, lua_tostring(L, 2));
					break;

				default:
					LuaScript_Unlock(_LS);
					luaL_error(L, "Unsupported value");
					return 0;
			}
			
			lua_rawseti(_LS->L, -2, (int)lua_objlen(_LS->L, -2) + 1);
			LuaScript_Unlock(_LS);
		}
	}

	return 0;
}

static int meta_avail(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, cont->name);
	lua_getfield(L, -1, "queue");
	lua_pushinteger(L, lua_objlen(L, -1));
	return 1;
}

static int meta_clear(lua_State *L) {
	struct _Contact *cont = checkcontact(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);
	lua_getfield(L, -1, cont->name);
	lua_getfield(L, -1, "queue");
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

	{"avail", meta_avail},
	{"clear", meta_clear},
	{"close", meta_close},

	{"__gc", meta_close},

	{NULL, NULL}
};

int luaopen_contact(lua_State *L) {
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, CSLUA_RCONTACT);

	lua_indexedmeta(L, CSLUA_MCONTACT, contactmeta);
	lua_pop(L, 1);

	luaL_newlib(L, contactlib);
	return 1;
}
