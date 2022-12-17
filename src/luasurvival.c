#include <core.h>
#include "luascript.h"

#ifdef CSLUA_USE_SURVIVAL
#include <plugin.h>
#include "luaclient.h"
#include "luasurvival.h"
#include "cs-survival/src/survitf.h"

extern SurvItf *SurvInterface;

static cs_bool surv_getdata(scr_Context *L, int idx, SrvData **data) {
	lua_getfield(L, LUA_REGISTRYINDEX, "_srvsafe");
	cs_bool safemode = (cs_bool)lua_toboolean(L, -1);
	lua_pop(L, 1);

	if(!safemode)
		luaL_argcheck(L, SurvInterface != NULL, 1, "SurvItf lost");
	else if(!SurvInterface)
		return false;

	*data = SurvInterface->getSrvData(lua_checkclient(L, idx));
	if(!safemode)
		luaL_argcheck(L, *data != NULL, 1, "No SurvData");

	return *data != NULL;
}

static int surv_isgod(scr_Context *L) {
	SrvData *data = NULL;
	lua_pushboolean(L,
		surv_getdata(L, 1, &data) &&
		SurvInterface->isInGodMode(data)
	);
	return 1;
}

static int surv_ispvp(scr_Context *L) {
	SrvData *data = NULL;
	lua_pushboolean(L,
		surv_getdata(L, 1, &data) &&
		SurvInterface->isInPvPMode(data)
	);
	return 1;
}

static int surv_setgod(scr_Context *L) {
	SrvData *data = NULL;
	cs_bool state = (cs_bool)lua_toboolean(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->setGodMode(data, state);
	return 0;
}

static int surv_giveblock(scr_Context *L) {
	BlockID id = (BlockID)luaL_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)luaL_checkinteger(L, 3);
	SrvData *data = NULL;
	lua_pushinteger(L,
		(lua_Integer)surv_getdata(L, 1, &data) &&
		SurvInterface->giveToInventory(data, id, ammount)
	);
	return 1;
}

static int surv_takeblock(scr_Context *L) {
	BlockID id = (BlockID)luaL_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)luaL_checkinteger(L, 3);
	SrvData *data = NULL;
	lua_pushinteger(L,
		(lua_Integer)surv_getdata(L, 1, &data) &&
		SurvInterface->takeFromInventory(data, id, ammount)
	);
	return 1;
}

static int surv_setpvp(scr_Context *L) {
	SrvData *data = NULL;
	cs_bool state = (cs_bool)lua_toboolean(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->setPvPMode(data, state);
	return 0;
}

static int surv_hurt(scr_Context *L) {
	SrvData *data = NULL;
	cs_byte dmg = (cs_byte)luaL_checkinteger(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->hurt(data, NULL, dmg);
	return 0;
}

static int surv_heal(scr_Context *L) {
	SrvData *data = NULL;
	cs_byte dmg = (cs_byte)luaL_checkinteger(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->heal(data, dmg);
	return 0;
}

static int surv_kill(scr_Context *L) {
	SrvData *data = NULL;
	if(surv_getdata(L, 1, &data))
		SurvInterface->kill(data);
	return 0;
}

static const luaL_Reg survivalmeta[] = {
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

INL static void surv_addfuncs(scr_Context *L) {
	luaL_getmetatable(L, CSLUA_MCLIENT);
	luaL_setfuncs(L, survivalmeta, 0);
	lua_pop(L, 1);
}

static int surv_init(scr_Context *L) {
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

static int surv_isready(scr_Context *L) {
	lua_pushboolean(L, SurvInterface != NULL);
	return 1;
}

static int surv_safe(scr_Context *L) {
	lua_pushboolean(L, lua_toboolean(L, 1));
	lua_setfield(L, LUA_REGISTRYINDEX, "_srvsafe");
	return 0;
}

static const luaL_Reg survlib[] = {
	{"init", surv_init},
	{"isready", surv_isready},
	{"safe", surv_safe},

	{NULL, NULL}
};

int luaopen_survival(scr_Context *L) {
	luaL_newlib(L, survlib);
	return 1;
}
#else
#warning "Compiling without the survival module"
int luaopen_survival(scr_Context *L) {
	lua_pushnil(L);
	return 1;
}
#endif
