#include <core.h>
#include <plugin.h>
#include "luaclient.h"
#include "luasurvival.h"
#include "cs-survival/src/survitf.h"

extern void *SurvInterface;

static SurvItf *surv_getitf(lua_State *L) {
	if(!SurvInterface) luaL_error(L, "SurvItf lost");
	return SurvInterface;
}

static int surv_isgod(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	lua_pushboolean(L, itf->isInGodMode(data));
	return 1;
}

static int surv_ispvp(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	lua_pushboolean(L, itf->isInPvPMode(data));
	return 1;
}

static int surv_setgod(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	luaL_checktype(L, 2, LUA_TBOOLEAN);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	itf->setGodMode(data, (cs_bool)lua_toboolean(L, 2));
	return 0;
}

static int surv_giveblock(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	BlockID id = (BlockID)luaL_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)luaL_checkinteger(L, 3);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	lua_pushinteger(L, (lua_Integer)itf->giveToInventory(data, id, ammount));
	return 1;
}

static int surv_takeblock(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	BlockID id = (BlockID)luaL_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)luaL_checkinteger(L, 3);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	lua_pushinteger(L, (lua_Integer)itf->takeFromInventory(data, id, ammount));
	return 1;
}

static int surv_setpvp(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	luaL_checktype(L, 2, LUA_TBOOLEAN);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	itf->setPvPMode(data, (cs_bool)lua_toboolean(L, 2));
	return 0;
}

static int surv_hurt(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_byte dmg = (cs_byte)luaL_checkinteger(L, 2);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	itf->hurt(data, NULL, dmg);
	return 0;
}

static int surv_heal(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_byte pts = (cs_byte)luaL_checkinteger(L, 2);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	itf->heal(data, pts);
	return 0;
}

static int surv_kill(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	SurvItf *itf = surv_getitf(L);
	SrvData *data = itf->getSrvData(client);
	itf->kill(data);
	return 0;
}

static luaL_Reg survivalmeta[] = {
	{"isingod", surv_isgod},
	{"isinpvp", surv_ispvp},

	{"setgod", surv_setgod},
	{"setpvp", surv_setpvp},

	{"giveblock", surv_giveblock},
	{"takeblock", surv_takeblock},

	{"hurt", surv_hurt},
	{"heal", surv_heal},
	{"kill", surv_kill},

	{NULL, NULL}
};

INL static void surv_addfuncs(lua_State *L) {
	luaL_getmetatable(L, CSLUA_MCLIENT);
	luaL_setfuncs(L, survivalmeta, 0);
	lua_pop(L, 1);
}

int surv_init(lua_State *L) {
	if(SurvInterface) {
		surv_addfuncs(L);
		lua_pushboolean(L, 1);
		return 1;
	}

	if(Plugin_RequestInterface(Plugin_RecvInterface, SURV_ITF_NAME)) {
		surv_addfuncs(L);
		lua_pushboolean(L, 1);
	} else
		lua_pushboolean(L, 0);

	return 1;
}

static luaL_Reg survlib[] = {
	{"init", surv_init},

	{NULL, NULL}
};

int luaopen_survival(lua_State *L) {
	luaL_register(L, luaL_checkstring(L, 1), survlib);
	return 1;
}
