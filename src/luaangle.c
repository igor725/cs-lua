#include <core.h>
#include <vector.h>
#include <platform.h>
#include "luaplugin.h"
#include "luaangle.h"

Ang *lua_checkangle(lua_State *L, int idx) {
	Ang *ud = luaL_checkudata(L, idx, "__anglemt");
	luaL_argcheck(L, ud != NULL, idx, "'Angle' expected");
	return ud;
}

static int ang_setyawvalue(lua_State *L) {
	Ang *ang = lua_checkangle(L, 1);
	ang->yaw = (cs_float)luaL_checknumber(L, 2);
	return 0;
}

static int ang_setpitchvalue(lua_State *L) {
	Ang *ang = lua_checkangle(L, 1);
	ang->pitch = (cs_float)luaL_checknumber(L, 2);
	return 0;
}

static int ang_setvalue(lua_State *L) {
	Ang *ang = lua_checkangle(L, 1);
	ang->yaw = (cs_float)luaL_checknumber(L, 2);
	ang->pitch = (cs_float)luaL_checknumber(L, 3);
	return 0;
}

static int ang_getyawvalue(lua_State *L) {
	Ang *ang = lua_checkangle(L, 1);
	lua_pushnumber(L, (lua_Number)ang->yaw);
	return 1;
}

static int ang_getpitchvalue(lua_State *L) {
	Ang *ang = lua_checkangle(L, 1);
	lua_pushnumber(L, (lua_Number)ang->pitch);
	return 1;
}

static int ang_getvalue(lua_State *L) {
	Ang *ang = lua_checkangle(L, 1);
	lua_pushnumber(L, (lua_Number)ang->yaw);
	lua_pushnumber(L, (lua_Number)ang->pitch);
	return 2;
}

static int ang_add(lua_State *L) {
	Ang *dst = lua_checkangle(L, 1);
	Ang *src = lua_checkangle(L, 2);
	dst->yaw += src->yaw;
	dst->pitch += src->pitch;
	lua_pushvalue(L, 1);
	return 1;
}

static int ang_sub(lua_State *L) {
	Ang *dst = lua_checkangle(L, 1);
	Ang *src = lua_checkangle(L, 2);
	dst->yaw -= src->yaw;
	dst->pitch -= src->pitch;
	lua_pushvalue(L, 1);
	return 1;
}

static int ang_mul(lua_State *L) {
	Ang *dst = lua_checkangle(L, 1);
	Ang *src = lua_checkangle(L, 2);
	dst->yaw *= src->yaw;
	dst->pitch *= src->pitch;
	lua_pushvalue(L, 1);
	return 1;
}

static int ang_div(lua_State *L) {
	Ang *dst = lua_checkangle(L, 1);
	Ang *src = lua_checkangle(L, 2);
	dst->yaw /= src->yaw;
	dst->pitch /= src->pitch;
	lua_pushvalue(L, 1);
	return 1;
}

static const luaL_Reg anglemeta[] = {
	{"setyaw", ang_setyawvalue},
	{"setpitch", ang_setpitchvalue},
	{"set", ang_setvalue},
	{"getx", ang_getyawvalue},
	{"gety", ang_getpitchvalue},
	{"get", ang_getvalue},
	{"__add", ang_add},
	{"__sub", ang_sub},
	{"__mul", ang_mul},
	{"__div", ang_div},
	{NULL, NULL}
};

static int ang_new(lua_State *L) {
	Ang *ang = lua_newuserdata(L, sizeof(Ang));
	Memory_Zero(&ang, sizeof(ang));
	luaL_setmetatable(L, "__anglemt");
	return 1;
}

static const luaL_Reg anglelib[] = {
	{"new", ang_new},
	{NULL, NULL}
};

int luaopen_angle(lua_State *L) {
	luaL_newmetatable(L, "__anglemt");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, anglemeta, 0);
	lua_pop(L, 1);

	luaL_register(L, "angle", anglelib);
	return 1;
}
