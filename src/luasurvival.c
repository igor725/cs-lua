#include <core.h>
#include "luascript.h"

#ifdef CSLUA_USE_SURVIVAL
#include <plugin.h>
#include "luaclient.h"
#include "luasurvival.h"
#include "cs-survival/src/survitf.h"

extern SurvItf *SurvInterface;

static cs_bool surv_getdata(lua_State *L, int idx, SrvData **data) {
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

static int surv_isgod(lua_State *L) {
	SrvData *data = NULL;
	lua_pushboolean(L,
		surv_getdata(L, 1, &data) &&
		SurvInterface->isInGodMode(data)
	);
	return 1;
}

static int surv_ispvp(lua_State *L) {
	SrvData *data = NULL;
	lua_pushboolean(L,
		surv_getdata(L, 1, &data) &&
		SurvInterface->isInPvPMode(data)
	);
	return 1;
}

static int surv_setgod(lua_State *L) {
	SrvData *data = NULL;
	cs_bool state = (cs_bool)lua_toboolean(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->setGodMode(data, state);
	return 0;
}

static int surv_giveblock(lua_State *L) {
	BlockID id = (BlockID)luaL_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)luaL_checkinteger(L, 3);
	SrvData *data = NULL;
	lua_pushinteger(L,
		(lua_Integer)surv_getdata(L, 1, &data) &&
		SurvInterface->giveToInventory(data, id, ammount)
	);
	return 1;
}

static int surv_takeblock(lua_State *L) {
	BlockID id = (BlockID)luaL_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)luaL_checkinteger(L, 3);
	SrvData *data = NULL;
	lua_pushinteger(L,
		(lua_Integer)surv_getdata(L, 1, &data) &&
		SurvInterface->takeFromInventory(data, id, ammount)
	);
	return 1;
}

static int surv_setpvp(lua_State *L) {
	SrvData *data = NULL;
	cs_bool state = (cs_bool)lua_toboolean(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->setPvPMode(data, state);
	return 0;
}

static int surv_hurt(lua_State *L) {
	SrvData *data = NULL;
	cs_byte dmg = (cs_byte)luaL_checkinteger(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->hurt(data, NULL, dmg);
	return 0;
}

static int surv_heal(lua_State *L) {
	SrvData *data = NULL;
	cs_byte dmg = (cs_byte)luaL_checkinteger(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->heal(data, dmg);
	return 0;
}

static int surv_kill(lua_State *L) {
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

INL static void surv_addfuncs(lua_State *L) {
	luaL_getmetatable(L, CSLUA_MCLIENT);
	luaL_setfuncs(L, survivalmeta, 0);
	lua_pop(L, 1);
}

static int surv_init(lua_State *L) {
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

static int surv_isready(lua_State *L) {
	lua_pushboolean(L, SurvInterface != NULL);
	return 1;
}

static int surv_safe(lua_State *L) {
	lua_pushboolean(L, lua_toboolean(L, 1));
	lua_setfield(L, LUA_REGISTRYINDEX, "_srvsafe");
	return 0;
}
#else
#define surv_safe surv_init
#define surv_isready surv_init

static int surv_init(lua_State *L) {
	lua_pushboolean(L, false);
	return 1;
}
#endif

static const luaL_Reg survlib[] = {
	{"init", surv_init},
	{"isready", surv_isready},
	{"safe", surv_safe},

	{NULL, NULL}
};

int luaopen_survival(lua_State *L) {
	luaL_register(L, luaL_checkstring(L, 1), survlib);
	return 1;
}
