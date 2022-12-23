#include "luascript.h"
#include "luavector.h"
#include <cpe.h>
#include <str.h>

CPEModel *scr_checkmodel(scr_Context *L, int idx) {
	return scr_checkmemtype(L, idx, CSSCRIPTS_MMODEL);
}

static int model_undefine(scr_Context *L) {
	scr_pushboolean(L, CPE_UndefineModelPtr(
		scr_checkmodel(L, 1)
	));
	return 1;
}

static const scr_RegFuncs modelmeta[] = {
	{"__gc", model_undefine},

	{NULL, NULL}
};

static void readModelPart(scr_Context *L, CPEModelPart *part) {
	if(scr_checktabfieldud(L, -1, "minCoords", CSSCRIPTS_MVECTOR))
		part->minCoords = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, -2, "maxCoords", CSSCRIPTS_MVECTOR))
		part->maxCoords = *scr_checkfloatvector(L, -1);
	if(scr_checktabfield(L, -3, "uvs", LUA_TTABLE)) {
		scr_stackcheck(L, 6 * 5);
		for(int i = 0; i < 6; i++) {
			scr_rawgeti(L, -1 - i * 5, i + 1);
			if(!scr_istable(L, -1))
				scr_fmterror(L, "Model UV #%d is not a table", i + 1);
			for(int j = 0; j < 4; j++) {
				scr_rawgeti(L, -1 - j, j + 1);
				if(!scr_isnumber(L, -1))
					scr_fmterror(L, "Model UV #%d has invalid format", i + 1);
				((cs_uint16 *)&part->UVs[i])[j] = (cs_uint16)scr_tointeger(L, -1);
			}
		}
		scr_stackpop(L, 6 * 5);
	}
	if(scr_checktabfieldud(L, -4, "rotAngles", CSSCRIPTS_MVECTOR))
		part->rotAngles = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, -5, "rotOrigin", CSSCRIPTS_MVECTOR))
		part->rotOrigin = *scr_checkfloatvector(L, -1);
	if(scr_checktabfield(L, -6, "anims", LUA_TTABLE)) {
		scr_stackcheck(L, 4 * 7);
		for(int i = 0; i < 4; i++) {
			scr_rawgeti(L, -1 - i * 7, i + 1);
			if(!scr_istable(L, -1))
				scr_fmterror(L, "Model anim #%d is not a table", i + 1);

			if(scr_checktabfield(L, -1, "flags", LUA_TNUMBER))
				part->anims[i].flags = (cs_byte)scr_tointeger(L, -1);
			if(scr_checktabfield(L, -2, "args", LUA_TTABLE)) {
				cs_float *args = &part->anims[i].a;
				for(int j = 0; j < 4; j++) {
					scr_rawgeti(L, -1 - j, j + 1);
					if(!scr_isnumber(L, -1))
						scr_fmterror(L, "Model anim #%d has invalid format", i);
					args[j] = (cs_float)scr_checknumber(L, -1);
				}
			}
		}
		scr_stackpop(L, 4 * 7);
	}
	if(scr_checktabfield(L, -7, "flags", LUA_TNUMBER))
		part->flags = (cs_byte)scr_tointeger(L, -1);

	scr_stackpop(L, 7);
}

static void parseModelParts(scr_Context *L, CPEModel *mdl) {
	scr_stackcheck(L, (int)mdl->partsCount);
	for(cs_byte i = 0; i < mdl->partsCount; i++) {
		scr_rawgeti(L, -2 - i, i + 1);
		if(!scr_istable(L, -1))
			scr_fmterror(L, "Model part #%d is not a table", i);
		readModelPart(L, &mdl->part[i]);
		if(i < mdl->partsCount - 1) // Устанавливаем ссылки на части модели
			mdl->part[i].next = &mdl->part[i + 1];
		else
			mdl->part[i].next = NULL;
	}
	scr_stackpop(L, mdl->partsCount);
}

static int model_create(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	scr_stackcheck(L, 10);
	CPEModel *mdl = NULL;

	if(scr_checktabfield(L, 1, "parts", LUA_TTABLE)) {
		cs_byte partsCount = (cs_byte)scr_gettablen(L, -1);
		if(!partsCount) scr_fmterror(L, "Model parts table can't be empty");
		if(partsCount > 64) scr_fmterror(L, "Maximum model parts number exceeded");
		mdl = scr_allocmem(L, sizeof(CPEModel) + sizeof(CPEModelPart) * partsCount);
		scr_setmemtype(L, CSSCRIPTS_MMODEL);
		mdl->partsCount = partsCount;
		mdl->part = (CPEModelPart *)&mdl[1]; // Указатель на конец CPEModel и начало CPEModelPart
		parseModelParts(L, mdl);
	}
	if(scr_checktabfield(L, 1, "name", LUA_TSTRING))
		String_Copy(mdl->name, sizeof(mdl->name), scr_tostring(L, -1));
	if(scr_checktabfield(L, 1, "flags", LUA_TNUMBER))
		mdl->flags = (cs_byte)scr_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "nameY", LUA_TNUMBER))
		mdl->nameY = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "eyeY", LUA_TNUMBER))
		mdl->eyeY = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfieldud(L, 1, "collideBox", CSSCRIPTS_MVECTOR))
		mdl->collideBox = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, 1, "clickMin", CSSCRIPTS_MVECTOR))
		mdl->clickMin = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, 1, "clickMax", CSSCRIPTS_MVECTOR))
		mdl->clickMax = *scr_checkfloatvector(L, -1);
	if(scr_checktabfield(L, 1, "uScale", LUA_TNUMBER))
		mdl->uScale = (cs_uint16)scr_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "vScale", LUA_TNUMBER))
		mdl->vScale = (cs_uint16)scr_tointeger(L, -1);

	scr_stackpop(L, 9);
	return 1;
}

static int model_define(scr_Context *L) {
	scr_pushboolean(L, CPE_DefineModel(
		(cs_byte)scr_checkinteger(L, 1),
		scr_checkmodel(L, 2)
	));
	return 1;
}

static int model_freeid(scr_Context *L) {
	cs_int16 id = -1;
	for(cs_byte i = 0; i < CPE_MAX_MODELS; i++) {
		if(!CPE_IsModelDefined(i)) {
			id = i;
			break;
		}
	}
	scr_pushinteger(L, (scr_Integer)id);
	return 1;
}

static const scr_RegFuncs modellib[] = {
	{"create", model_create},
	{"define", model_define},
	{"undefine", model_undefine},
	{"freeid", model_freeid},

	{NULL, NULL}
};

int scr_libfunc(model)(scr_Context *L) {
	scr_createtype(L, CSSCRIPTS_MMODEL, modelmeta);
	scr_newlib(L, modellib);
	return 1;
}
