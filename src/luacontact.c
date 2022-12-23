#include <core.h>
#include <str.h>
#include "luascript.h"
#include "luacontact.h"
#include "luaclient.h"
#include "luaworld.h"

static struct _Contact {
	cs_char name[CSSCRIPTS_CONTACT_NAMELEN];
	Script *scripts[CSSCRIPTS_CONTACT_MAXSCRIPTS];
} contacts[CSSCRIPTS_CONTACT_MAX] = {0};

static struct _Contact *newcontact(cs_str name, Script *LS) {
	for(int i = 0; i < CSSCRIPTS_CONTACT_MAX; i++) {
		struct _Contact *cont = &contacts[i];
		if(*cont->name == '\0') {
			String_Copy(cont->name, CSSCRIPTS_CONTACT_NAMELEN, name);
			cont->scripts[0] = LS;
			return cont;
		}
	}

	return NULL;
}

static struct _Contact *checkcontact(scr_Context *L, int idx) {
	struct _Contact *cont = *(void **)scr_checkmemtype(L, idx, CSSCRIPTS_MCONTACT);
	scr_argassert(L, *cont->name != '\0', idx, "Contact closed");
	return cont;
}

static struct _Contact *tocontact(scr_Context *L, int idx) {
	return *(void **)scr_checkmemtype(L, idx, CSSCRIPTS_MCONTACT);
}

static cs_bool addcontactscript(struct _Contact *cont, Script *LS) {
	for(int i = 0; i < CSSCRIPTS_CONTACT_MAXSCRIPTS; i++) {
		if(!cont->scripts[i]) {
			cont->scripts[i] = LS;
			return true;
		}
	}

	return false;
}

static void pushcontact(scr_Context *L, struct _Contact *cont) {
	if(!cont) {
		scr_pushnull(L);
		return;
	}

	void **ud = scr_allocmem(L, sizeof(cont));
	scr_setmemtype(L, CSSCRIPTS_MCONTACT);
	*ud = cont;
}

static int meta_pop(scr_Context *L) {
	struct _Contact *cont = checkcontact(L, 1);
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);
	scr_gettabfield(L, -1, cont->name);
	scr_rawgeti(L, -1, 0);
	int tablen = (int)scr_gettablen(L, -1);
	if(tablen > 0) {
		scr_rawgeti(L, -1, 1);
		for(int pos = 1; pos <= tablen; pos++) {
			scr_rawgeti(L, -2, pos + 1);
			scr_rawseti(L, -3, pos);
		}
		return 1;
	}

	scr_pushnull(L);
	return 1;
}

static void copytable(scr_Context *L_to, scr_Context *L_from, int idx, cs_str *errt);

static void copyudata(scr_Context *L_to, scr_Context *L_from, int idx, cs_str *errt) {
	void *tmp = scr_toclient(L_from, idx);
	if(tmp) {
		scr_pushclient(L_to, tmp);
		return;
	}
	tmp = scr_toworld(L_from, idx);
	if(tmp) {
		scr_pushworld(L_to, tmp);
		return;
	}

	if(scr_getmetafield(L_from, idx, "__name"))
		*errt = scr_tostring(L_from, -1);
	else
		*errt = luaL_typename(L_from, idx);

	return;
}

static int writer(scr_Context *L, const void *b, size_t size, void *B) {
	(void)L;
	luaL_addlstring((luaL_Buffer *)B, (const char *)b, size);
	return 0;
}

static const char *reader(scr_Context *L, void *b, size_t *size) {
	(void)b;
	return lua_tolstring(L, -1, size);
}

static void copyfunction(scr_Context *L_to, scr_Context *L_from, int idx, cs_str *errt) {
	if(scr_isnativefunc(L_from, idx)) {
		*errt = "C function";
		return;
	} else
		*errt = scr_typename(L_from, idx);

	luaL_Buffer b;
	luaL_buffinit(L_to, &b);
	scr_stackpush(L_from, idx);
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

	scr_stackrem(L_to, -2);
	*errt = NULL;
}

static cs_bool copyvalue(scr_Context *L_to, scr_Context *L_from, int idx, cs_str *errt) {
	switch(lua_type(L_from, idx)) {
		case LUA_TNIL:
			scr_pushnull(L_to);
			break;

		case LUA_TBOOLEAN:
			scr_pushboolean(L_to, scr_toboolean(L_from, idx));
			break;

		case LUA_TLIGHTUSERDATA:
			scr_pushptr(L_to, scr_getmem(L_from, idx));
			break;

		case LUA_TNUMBER:
			scr_pushnumber(L_to, scr_tonumber(L_from, idx));
			break;

		case LUA_TSTRING:
			scr_pushstring(L_to, scr_tostring(L_from, idx));
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

static void copytable(scr_Context *L_to, scr_Context *L_from, int idx, cs_str *errt) {
	// Превращаем относительный индекс в абсолютный
	if(idx < 0) idx = lua_absindex(L_from, idx);

	scr_pushnull(L_from);
	scr_newtable(L_to);
	while(lua_next(L_from, idx) != 0) {
		if(scr_raweq(L_from, idx, -2)) // Если ключ сама же эта таблица, то её и пушим
			scr_stackpush(L_to, -1);
		else if(!copyvalue(L_to, L_from, -2, errt))
			return;

		if(scr_raweq(L_from, idx, -1))
			scr_stackpush(L_to, -2);
		else if(!copyvalue(L_to, L_from, -1, errt))
			return;

		scr_rawset(L_to, -3);
		scr_stackpop(L_from, 1);
	}

	return;
}

static int meta_push(scr_Context *L) {
	struct _Contact *cont = checkcontact(L, 1);
	cs_str errt = NULL;

	for(int i = 0; i < CSSCRIPTS_CONTACT_MAXSCRIPTS; i++) {
		Script *_LS = cont->scripts[i];
		if(!_LS) continue;

		if(_LS->L != L) {
			Script_Lock(_LS);
			int tstart = scr_stacktop(_LS->L);
			scr_gettabfield(_LS->L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);
			scr_gettabfield(_LS->L, -1, cont->name);
			scr_rawgeti(_LS->L, -1, 0);
			if(!copyvalue(_LS->L, L, 2, &errt)) {
				scr_stackset(_LS->L, tstart);
				Script_Unlock(_LS);
				scr_fmterror(L, "Attempt to push an unsupported value: %s", errt);
				return 0;
			}
			scr_rawseti(_LS->L, -2, (int)scr_gettablen(_LS->L, -2) + 1);
			scr_rawgeti(_LS->L, -2, 2);
			if(scr_isfunc(_LS->L, -1)) {
				scr_rawgeti(_LS->L, -3, 1);
				(void)Script_Call(_LS, 1, 0);
			}
			scr_stackset(_LS->L, tstart);
			Script_Unlock(_LS);
		}
	}

	return 0;
}

static int meta_bind(scr_Context *L) {
	luaL_checktype(L, 2, LUA_TFUNCTION);
	struct _Contact *cont = checkcontact(L, 1);
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);
	scr_gettabfield(L, -1, cont->name);
	scr_stackpush(L, 2);
	scr_rawseti(L, -2, 2); // cscontact[name][2] = <2>
	return 0;
}

static int meta_avail(scr_Context *L) {
	struct _Contact *cont = checkcontact(L, 1);
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);
	scr_gettabfield(L, -1, cont->name);
	scr_rawgeti(L, -1, 0);
	scr_pushinteger(L, scr_gettablen(L, -1));
	return 1;
}

static int meta_clear(scr_Context *L) {
	struct _Contact *cont = checkcontact(L, 1);
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);
	scr_gettabfield(L, -1, cont->name);
	scr_rawgeti(L, -1, 0);
	for(int i = (int)scr_gettablen(L, -1); i > 0; i--) {
		scr_pushnull(L);
		scr_rawseti(L, -2, i);
	}
	return 0;
}

static int meta_close(scr_Context *L) {
	struct _Contact *cont = tocontact(L, 1);
	if(!cont || *cont->name == '\0') return 0;
	cs_bool dirty = false;

	for(int i = 0; i < CSSCRIPTS_CONTACT_MAXSCRIPTS; i++) {
		Script *_LS = cont->scripts[i];
		if(_LS) {
			if(_LS->L == L) {
				cont->scripts[i] = NULL;
				continue;
			}
			dirty = true;
		}
	}

	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);
	scr_pushnull(L);
	scr_settabfield(L, -2, cont->name);

	if(!dirty)
		*cont->name = '\0';

	return 0;
}

const scr_RegFuncs contactmeta[] = {
	{"pop", meta_pop},
	{"push", meta_push},
	{"bind", meta_bind},

	{"avail", meta_avail},
	{"clear", meta_clear},
	{"close", meta_close},

	{"__gc", meta_close},

	{NULL, NULL}
};

static int cont_get(scr_Context *L) {
	cs_size namelen = 0;
	cs_str name = luaL_checklstring(L, 1, &namelen);
	scr_argassert(L, namelen > 0, 1, "Empty contact name");
	scr_argassert(L, namelen < CSSCRIPTS_CONTACT_NAMELEN, 1, "Too long contact name");
	Script *LS = Script_GetHandle(L);

	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);
	scr_gettabfield(L, -1, name); // cscontact[name]
	if(scr_isnull(L, -1)) {
		scr_stackpop(L, 1);
		for(int i = 0; i < CSSCRIPTS_CONTACT_MAX; i++) {
			struct _Contact *cont = &contacts[i];
			if(String_Compare(cont->name, name)) {
				if(addcontactscript(cont, LS)) {
					pushcontact(L, cont);
					goto cfinish;
				} else
					scr_fmterror(L, "Failed to connect to specified contact");
				return 1;
			}
		}

		pushcontact(L, newcontact(name, LS));

		cfinish:
		scr_newtable(L);
		scr_newtable(L);
		scr_rawseti(L, -2, 0);
		scr_stackpush(L, -2); // Contact
		scr_rawseti(L, -2, 1);
		scr_settabfield(L, -3, name);
		return 1;
	}

	scr_rawgeti(L, -1, 1);
	return 1;
}

const scr_RegFuncs contactlib[] = {
	{"get", cont_get},

	{NULL, NULL}
};

int scr_libfunc(contact)(scr_Context *L) {
	scr_newntable(L, 0, CSSCRIPTS_CONTACT_MAX);
	scr_settabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCONTACT);

	scr_createtype(L, CSSCRIPTS_MCONTACT, contactmeta);
	scr_newlib(L, contactlib);
	return 1;
}
