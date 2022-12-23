#include <core.h>
#include "luascript.h"

#ifdef CSSCRIPTS_USE_SURVIVAL
#include <plugin.h>
#include "luaclient.h"
#include "luasurvival.h"
#include "cs-survival/src/survitf.h"

extern SurvItf *SurvInterface;

static cs_bool surv_getdata(scr_Context *L, int idx, SrvData **data) {
	scr_gettabfield(L, LUA_REGISTRYINDEX, "_srvsafe");
	cs_bool safemode = scr_toboolean(L, -1);
	scr_stackpop(L, 1);

	if(!safemode)
		scr_argassert(L, SurvInterface != NULL, 1, "SurvItf lost");
	else if(!SurvInterface)
		return false;

	*data = SurvInterface->getSrvData(scr_checkclient(L, idx));
	if(!safemode)
		scr_argassert(L, *data != NULL, 1, "No SurvData");

	return *data != NULL;
}

static int surv_isgod(scr_Context *L) {
	SrvData *data = NULL;
	scr_pushboolean(L,
		surv_getdata(L, 1, &data) &&
		SurvInterface->isInGodMode(data)
	);
	return 1;
}

static int surv_ispvp(scr_Context *L) {
	SrvData *data = NULL;
	scr_pushboolean(L,
		surv_getdata(L, 1, &data) &&
		SurvInterface->isInPvPMode(data)
	);
	return 1;
}

static int surv_setgod(scr_Context *L) {
	SrvData *data = NULL;
	cs_bool state = scr_toboolean(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->setGodMode(data, state);
	return 0;
}

static int surv_giveblock(scr_Context *L) {
	BlockID id = (BlockID)scr_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)scr_checkinteger(L, 3);
	SrvData *data = NULL;
	scr_pushinteger(L,
		(scr_Integer)surv_getdata(L, 1, &data) &&
		SurvInterface->giveToInventory(data, id, ammount)
	);
	return 1;
}

static int surv_takeblock(scr_Context *L) {
	BlockID id = (BlockID)scr_checkinteger(L, 2);
	cs_uint16 ammount = (cs_uint16)scr_checkinteger(L, 3);
	SrvData *data = NULL;
	scr_pushinteger(L,
		(scr_Integer)surv_getdata(L, 1, &data) &&
		SurvInterface->takeFromInventory(data, id, ammount)
	);
	return 1;
}

static int surv_setpvp(scr_Context *L) {
	SrvData *data = NULL;
	cs_bool state = scr_toboolean(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->setPvPMode(data, state);
	return 0;
}

static int surv_hurt(scr_Context *L) {
	SrvData *data = NULL;
	cs_byte dmg = (cs_byte)scr_checkinteger(L, 2);
	if(surv_getdata(L, 1, &data))
		SurvInterface->hurt(data, NULL, dmg);
	return 0;
}

static int surv_heal(scr_Context *L) {
	SrvData *data = NULL;
	cs_byte dmg = (cs_byte)scr_checkinteger(L, 2);
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

static const scr_RegFuncs survivalmeta[] = {
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
	luaL_getmetatable(L, CSSCRIPTS_MCLIENT);
	luaL_setfuncs(L, survivalmeta, 0);
	scr_stackpop(L, 1);
}

static int surv_init(scr_Context *L) {
	if(SurvInterface) {
		surv_addfuncs(L);
		scr_pushboolean(L, 1);
		return 1;
	}

	if(Plugin_RequestInterface(Plugin_RecvInterface, SURV_ITF_NAME)) {
		surv_addfuncs(L);
		scr_pushboolean(L, 1);
	} else
		scr_pushboolean(L, 0);

	return 1;
}

static int surv_isready(scr_Context *L) {
	scr_pushboolean(L, SurvInterface != NULL);
	return 1;
}

static int surv_safe(scr_Context *L) {
	scr_pushboolean(L, scr_toboolean(L, 1));
	scr_settabfield(L, LUA_REGISTRYINDEX, "_srvsafe");
	return 0;
}

static const scr_RegFuncs survlib[] = {
	{"init", surv_init},
	{"isready", surv_isready},
	{"safe", surv_safe},

	{NULL, NULL}
};

int scr_libfunc(survival)(scr_Context *L) {
	scr_newlib(L, survlib);
	return 1;
}
#else
#warning "Compiling without the survival module"
int scr_libfunc(survival)(scr_Context *L) {
	scr_pushnull(L);
	return 1;
}
#endif
