#include "luascript.h"
#include "luavector.h"
#include <protocol.h>
#include <str.h>

static const luaL_Reg modelmeta[] = {
	{NULL, NULL}
};

static void readModelPart(lua_State *L, CPEModelPart *part) {
	(void)L; (void)part;
	// TODO: Part reading
}

static void parseModelParts(lua_State *L, CPEModel *mdl) {
	int partsCount = (int)lua_objlen(L, -1);
	CPEModelPart *parts = lua_newuserdata(L, partsCount * sizeof(CPEModelPart));
	for(int i = 1; i <= partsCount; i++) {
		lua_pushinteger(L, i);
		lua_gettable(L, -2);
		if(!lua_istable(L, -1)) {
			luaL_error(L, "Model part #%d is not a table", i);
			return;
		}
		readModelPart(L, &parts[i]);
		lua_pop(L, 1);
	}
	mdl->part = parts;
}

static int model_create(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	CPEModel *mdl = lua_newuserdata(L, sizeof(CPEModel));
	luaL_setmetatable(L, CSLUA_MMODEL);

	if(lua_checktabfield(L, 1, "name", LUA_TSTRING))
		String_Copy(mdl->name, sizeof(mdl->name), lua_tostring(L, -1));
	if(lua_checktabfield(L, 1, "flags", LUA_TNUMBER))
		mdl->flags = (cs_byte)lua_tointeger(L, -1);
	if(lua_checktabfield(L, 1, "nameY", LUA_TNUMBER))
		mdl->nameY = (cs_byte)lua_tonumber(L, -1);
	if(lua_checktabfield(L, 1, "eyeY", LUA_TNUMBER))
		mdl->eyeY = (cs_byte)lua_tonumber(L, -1);
	if(lua_checktabfieldud(L, 1, "collideBox", CSLUA_MVECTOR))
		mdl->collideBox = *lua_checkfloatvector(L, -1);
	if(lua_checktabfieldud(L, 1, "clickMin", CSLUA_MVECTOR))
		mdl->clickMin = *lua_checkfloatvector(L, -1);
	if(lua_checktabfieldud(L, 1, "clickMax", CSLUA_MVECTOR))
		mdl->clickMax = *lua_checkfloatvector(L, -1);
	if(lua_checktabfield(L, 1, "uScale", LUA_TNUMBER))
		mdl->uScale = (cs_uint16)lua_tointeger(L, -1);
	if(lua_checktabfield(L, 1, "vScale", LUA_TNUMBER))
		mdl->vScale = (cs_uint16)lua_tointeger(L, -1);
	if(lua_checktabfield(L, 1, "parts", LUA_TTABLE))
		parseModelParts(L, mdl);
	
	lua_pop(L, 10);
	return 1;
}

static const luaL_Reg modellib[] = {
	{"create", model_create},

	{NULL, NULL}
};

int luaopen_model(lua_State *L) {
	luaL_newmetatable(L, CSLUA_MMODEL);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, modelmeta, 0);
	lua_pop(L, 1);

	luaL_register(L, luaL_checkstring(L, 1), modellib);
	return 1;
}
