#include <core.h>
#include <plugin.h>
#include "luaclient.h"
#include "luasurvival.h"
#include "cs-survival/src/survitf.h"

static SurvItf *surv_getitf(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "SurvItf");
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return NULL;
	}
	luaL_checktype(L, -1, LUA_TLIGHTUSERDATA);
	SurvItf *ptr = lua_touserdata(L, -1);
	lua_pop(L, 1);
	return ptr;
}

static int surv_isgod(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	SurvItf *itf = surv_getitf(L);
	if(itf) {
		SrvData *data = itf->getSrvData(client);
		lua_pushboolean(L, itf->isInGodMode(data));
	} else lua_pushboolean(L, 0);
	return 1;
}

static int surv_ispvp(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	SurvItf *itf = surv_getitf(L);
	if(itf) {
		SrvData *data = itf->getSrvData(client);
		lua_pushboolean(L, itf->isInPvPMode(data));
	} else lua_pushboolean(L, 0);
	return 1;
}

static int surv_setgod(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	luaL_checktype(L, 2, LUA_TBOOLEAN);
	SurvItf *itf = surv_getitf(L);
	if(itf) {
		SrvData *data = itf->getSrvData(client);
		itf->setGodMode(data, (cs_bool)lua_toboolean(L, 2));
		lua_pushboolean(L, 1);
	} else lua_pushboolean(L, 0);
	return 1;
}

static int surv_setpvp(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	luaL_checktype(L, 2, LUA_TBOOLEAN);
	SurvItf *itf = surv_getitf(L);
	if(itf) {
		SrvData *data = itf->getSrvData(client);
		itf->setPvPMode(data, (cs_bool)lua_toboolean(L, 2));
		lua_pushboolean(L, 1);
	} else lua_pushboolean(L, 0);
	return 1;
}

static int surv_hurt(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_byte dmg = (cs_byte)luaL_checkinteger(L, 2);
	SurvItf *itf = surv_getitf(L);
	if(itf) {
		SrvData *data = itf->getSrvData(client);
		itf->hurt(data, NULL, dmg);
		lua_pushboolean(L, 1);
		return 1;
	}

	lua_pushboolean(L, 0);
	return 1;
}

static int surv_heal(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_byte pts = (cs_byte)luaL_checkinteger(L, 2);
	SurvItf *itf = surv_getitf(L);
	if(itf) {
		SrvData *data = itf->getSrvData(client);
		itf->heal(data, pts);
		lua_pushboolean(L, 1);
		return 1;
	}

	lua_pushboolean(L, 0);
	return 1;
}

static int surv_kill(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	SurvItf *itf = surv_getitf(L);
	if(itf) {
		SrvData *data = itf->getSrvData(client);
		itf->kill(data);
		lua_pushboolean(L, 1);
		return 1;
	}

	lua_pushboolean(L, 0);
	return 1;
}

static luaL_Reg survivalmeta[] = {
	{"isingod", surv_isgod},
	{"isinpvp", surv_ispvp},

	{"setgod", surv_setgod},
	{"setpvp", surv_setpvp},

	{"hurt", surv_hurt},
	{"heal", surv_heal},
	{"kill", surv_kill},

	{NULL, NULL}
};

int luasurv_request(lua_State *L) {
	if(Plugin_RequestInterface(Plugin_RecvInterface, SURV_ITF_NAME)) {
		luaL_getmetatable(L, "Client");
		luaL_setfuncs(L, survivalmeta, 0);
		lua_pop(L, 1);
		lua_pushboolean(L, 1);
	} else lua_pushboolean(L, 0);
	return 1;
}
