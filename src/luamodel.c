#include "luascript.h"
#include "luavector.h"
#include <cpe.h>
#include <str.h>

CPEModel *scr_checkmodel(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, CSSCRIPTS_MMODEL);
}

static int model_undefine(lua_State *L) {
	lua_pushboolean(L, CPE_UndefineModelPtr(
		scr_checkmodel(L, 1)
	));
	return 1;
}

static const luaL_Reg modelmeta[] = {
	{"__gc", model_undefine},

	{NULL, NULL}
};

static void readModelPart(lua_State *L, CPEModelPart *part) {
	if(scr_checktabfieldud(L, -1, "minCoords", CSSCRIPTS_MVECTOR))
		part->minCoords = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, -2, "maxCoords", CSSCRIPTS_MVECTOR))
		part->maxCoords = *scr_checkfloatvector(L, -1);
	if(scr_checktabfield(L, -3, "uvs", LUA_TTABLE)) {
		lua_checkstack(L, 6 * 5);
		for(int i = 0; i < 6; i++) {
			lua_rawgeti(L, -1 - i * 5, i + 1);
			if(!lua_istable(L, -1))
				luaL_error(L, "Model UV #%d is not a table", i + 1);
			for(int j = 0; j < 4; j++) {
				lua_rawgeti(L, -1 - j, j + 1);
				if(!lua_isnumber(L, -1))
					luaL_error(L, "Model UV #%d has invalid format", i + 1);
				((cs_uint16 *)&part->UVs[i])[j] = (cs_uint16)lua_tointeger(L, -1);
			}
		}
		lua_pop(L, 6 * 5);
	}
	if(scr_checktabfieldud(L, -4, "rotAngles", CSSCRIPTS_MVECTOR))
		part->rotAngles = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, -5, "rotOrigin", CSSCRIPTS_MVECTOR))
		part->rotOrigin = *scr_checkfloatvector(L, -1);
	if(scr_checktabfield(L, -6, "anims", LUA_TTABLE)) {
		lua_checkstack(L, 4 * 7);
		for(int i = 0; i < 4; i++) {
			lua_rawgeti(L, -1 - i * 7, i + 1);
			if(!lua_istable(L, -1))
				luaL_error(L, "Model anim #%d is not a table", i + 1);

			if(scr_checktabfield(L, -1, "flags", LUA_TNUMBER))
				part->anims[i].flags = (cs_byte)lua_tointeger(L, -1);
			if(scr_checktabfield(L, -2, "args", LUA_TTABLE)) {
				cs_float *args = &part->anims[i].a;
				for(int j = 0; j < 4; j++) {
					lua_rawgeti(L, -1 - j, j + 1);
					if(!lua_isnumber(L, -1))
						luaL_error(L, "Model anim #%d has invalid format", i);
					args[j] = (cs_float)luaL_checknumber(L, -1);
				}
			}
		}
		lua_pop(L, 4 * 7);
	}
	if(scr_checktabfield(L, -7, "flags", LUA_TNUMBER))
		part->flags = (cs_byte)lua_tointeger(L, -1);

	lua_pop(L, 7);
}

static void parseModelParts(lua_State *L, CPEModel *mdl) {
	lua_checkstack(L, (int)mdl->partsCount);
	for(cs_byte i = 0; i < mdl->partsCount; i++) {
		lua_rawgeti(L, -2 - i, i + 1);
		if(!lua_istable(L, -1))
			luaL_error(L, "Model part #%d is not a table", i);
		readModelPart(L, &mdl->part[i]);
		if(i < mdl->partsCount - 1) // Устанавливаем ссылки на части модели
			mdl->part[i].next = &mdl->part[i + 1];
		else
			mdl->part[i].next = NULL;
	}
	lua_pop(L, mdl->partsCount);
}

static int model_create(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_checkstack(L, 10);
	CPEModel *mdl = NULL;

	if(scr_checktabfield(L, 1, "parts", LUA_TTABLE)) {
		cs_byte partsCount = (cs_byte)lua_objlen(L, -1);
		if(!partsCount) luaL_error(L, "Model parts table can't be empty");
		if(partsCount > 64) luaL_error(L, "Maximum model parts number exceeded");
		mdl = lua_newuserdata(L, sizeof(CPEModel) + sizeof(CPEModelPart) * partsCount);
		luaL_setmetatable(L, CSSCRIPTS_MMODEL);
		mdl->partsCount = partsCount;
		mdl->part = (CPEModelPart *)&mdl[1]; // Указатель на конец CPEModel и начало CPEModelPart
		parseModelParts(L, mdl);
	}
	if(scr_checktabfield(L, 1, "name", LUA_TSTRING))
		String_Copy(mdl->name, sizeof(mdl->name), lua_tostring(L, -1));
	if(scr_checktabfield(L, 1, "flags", LUA_TNUMBER))
		mdl->flags = (cs_byte)lua_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "nameY", LUA_TNUMBER))
		mdl->nameY = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "eyeY", LUA_TNUMBER))
		mdl->eyeY = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfieldud(L, 1, "collideBox", CSSCRIPTS_MVECTOR))
		mdl->collideBox = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, 1, "clickMin", CSSCRIPTS_MVECTOR))
		mdl->clickMin = *scr_checkfloatvector(L, -1);
	if(scr_checktabfieldud(L, 1, "clickMax", CSSCRIPTS_MVECTOR))
		mdl->clickMax = *scr_checkfloatvector(L, -1);
	if(scr_checktabfield(L, 1, "uScale", LUA_TNUMBER))
		mdl->uScale = (cs_uint16)lua_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "vScale", LUA_TNUMBER))
		mdl->vScale = (cs_uint16)lua_tointeger(L, -1);

	lua_pop(L, 9);
	return 1;
}

static int model_define(lua_State *L) {
	lua_pushboolean(L, CPE_DefineModel(
		(cs_byte)luaL_checkinteger(L, 1),
		scr_checkmodel(L, 2)
	));
	return 1;
}

static int model_freeid(lua_State *L) {
	cs_int16 id = -1;
	for(cs_byte i = 0; i < CPE_MAX_MODELS; i++) {
		if(!CPE_IsModelDefined(i)) {
			id = i;
			break;
		}
	}
	lua_pushinteger(L, (lua_Integer)id);
	return 1;
}

static const luaL_Reg modellib[] = {
	{"create", model_create},
	{"define", model_define},
	{"undefine", model_undefine},
	{"freeid", model_freeid},

	{NULL, NULL}
};

int scr_libfunc(model)(lua_State *L) {
	scr_createtype(L, CSSCRIPTS_MMODEL, modelmeta);
	luaL_newlib(L, modellib);
	return 1;
}
