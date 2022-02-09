#include <core.h>
#include <vector.h>
#include <platform.h>
#include "luaplugin.h"
#include "luavector.h"

LuaVector *lua_newluavector(lua_State *L) {
	LuaVector *vec = lua_newuserdata(L, sizeof(LuaVector));
	Memory_Zero(&vec->value, sizeof(vec->value));
	luaL_setmetatable(L, "Vector");
	return vec;
}

LuaVector *lua_checkvector(lua_State *L, int idx) {
	return (LuaVector *)luaL_checkudata(L, idx, "Vector");
}

Vec *lua_checkfloatvector(lua_State *L, int idx) {
	LuaVector *vec = lua_checkvector(L, idx);
	luaL_argcheck(L, vec->type == 0, idx, "'FloatVector' expected");
	return &vec->value.f;
}

SVec *lua_checkshortvector(lua_State *L, int idx) {
	LuaVector *vec = lua_checkvector(L, idx);
	luaL_argcheck(L, vec->type == 1, idx, "'ShortVector' expected");
	return &vec->value.s;
}

static int vec_iszero(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0)
		lua_pushboolean(L, Vec_IsZero(vec->value.f));
	else if(vec->type == 1)
		lua_pushboolean(L, Vec_IsZero(vec->value.s));

	return 1;
}

static int vec_scale(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0) {
		Vec_Scale(vec->value.f, (cs_float)luaL_checknumber(L, 2));
	} else if(vec->type == 1) {
		Vec_Scale(vec->value.s, (cs_int16)luaL_checkinteger(L, 2));
	}

	return 0;
}

static int vec_setxvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0)
		vec->value.f.x = (cs_float)luaL_checknumber(L, 2);
	else if(vec->type == 1)
		vec->value.s.x = (cs_int16)luaL_checkinteger(L, 2);

	return 0;
}

static int vec_setyvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0)
		vec->value.f.y = (cs_float)luaL_checknumber(L, 2);
	else if(vec->type == 1)
		vec->value.s.y = (cs_int16)luaL_checkinteger(L, 2);

	return 0;
}

static int vec_setzvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0)
		vec->value.f.z = (cs_float)luaL_checknumber(L, 2);
	else if(vec->type == 1)
		vec->value.s.z = (cs_int16)luaL_checkinteger(L, 2);

	return 0;
}

static int vec_setvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0) {
		vec->value.f.x = (cs_float)luaL_checknumber(L, 2);
		vec->value.f.y = (cs_float)luaL_checknumber(L, 3);
		vec->value.f.z = (cs_float)luaL_checknumber(L, 4);
	} else if(vec->type == 1) {
		vec->value.s.x = (cs_int16)luaL_checkinteger(L, 2);
		vec->value.s.y = (cs_int16)luaL_checkinteger(L, 3);
		vec->value.s.z = (cs_int16)luaL_checkinteger(L, 4);
	}

	return 0;
}

static int vec_getxvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0)
		lua_pushnumber(L, (lua_Number)vec->value.f.x);
	else if(vec->type == 1)
		lua_pushinteger(L, (lua_Integer)vec->value.s.x);
	else return 0;

	return 1;
}

static int vec_getyvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0)
		lua_pushnumber(L, (lua_Number)vec->value.f.y);
	else if(vec->type == 1)
		lua_pushinteger(L, (lua_Integer)vec->value.s.y);
	else return 0;

	return 1;
}

static int vec_getzvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0)
		lua_pushnumber(L, (lua_Number)vec->value.f.z);
	else if(vec->type == 1)
		lua_pushinteger(L, (lua_Integer)vec->value.s.z);
	else return 0;

	return 1;
}

static int vec_getvalue(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	if(vec->type == 0) {
		lua_pushnumber(L, (lua_Number)vec->value.f.x);
		lua_pushnumber(L, (lua_Number)vec->value.f.y);
		lua_pushnumber(L, (lua_Number)vec->value.f.z);
	} else if(vec->type == 1) {
		lua_pushinteger(L, (lua_Integer)vec->value.s.x);
		lua_pushinteger(L, (lua_Integer)vec->value.s.y);
		lua_pushinteger(L, (lua_Integer)vec->value.s.z);
	} else return 0;

	return 3;
}

static int vec_add(lua_State *L) {
	LuaVector *dst = lua_checkvector(L, 1);
	LuaVector *src = lua_checkvector(L, 2);

	if(dst->type == 0) {
		dst->value.f.x += src->type == 0 ? src->value.f.x : (cs_float)src->value.s.x;
		dst->value.f.y += src->type == 0 ? src->value.f.y : (cs_float)src->value.s.y;
		dst->value.f.z += src->type == 0 ? src->value.f.z : (cs_float)src->value.s.z;
	} else if(dst->type == 1) {
		dst->value.s.x += src->type == 1 ? src->value.s.x : (cs_int16)src->value.f.x;
		dst->value.s.y += src->type == 1 ? src->value.s.y : (cs_int16)src->value.f.y;
		dst->value.s.z += src->type == 1 ? src->value.s.z : (cs_int16)src->value.f.z;
	}

	lua_pushvalue(L, 1);
	return 1;
}

static int vec_sub(lua_State *L) {
	LuaVector *dst = lua_checkvector(L, 1);
	LuaVector *src = lua_checkvector(L, 2);

	if(dst->type == 0) {
		dst->value.f.x -= src->type == 0 ? src->value.f.x : (cs_float)src->value.s.x;
		dst->value.f.y -= src->type == 0 ? src->value.f.y : (cs_float)src->value.s.y;
		dst->value.f.z -= src->type == 0 ? src->value.f.z : (cs_float)src->value.s.z;
	} else if(dst->type == 1) {
		dst->value.s.x -= src->type == 1 ? src->value.s.x : (cs_int16)src->value.f.x;
		dst->value.s.y -= src->type == 1 ? src->value.s.y : (cs_int16)src->value.f.y;
		dst->value.s.z -= src->type == 1 ? src->value.s.z : (cs_int16)src->value.f.z;
	}

	lua_pushvalue(L, 1);
	return 1;
}

static int vec_mul(lua_State *L) {
	LuaVector *dst = lua_checkvector(L, 1);
	LuaVector *src = lua_checkvector(L, 2);

	if(dst->type == 0) {
		dst->value.f.x *= src->type == 0 ? src->value.f.x : (cs_float)src->value.s.x;
		dst->value.f.y *= src->type == 0 ? src->value.f.y : (cs_float)src->value.s.y;
		dst->value.f.z *= src->type == 0 ? src->value.f.z : (cs_float)src->value.s.z;
	} else if(dst->type == 1) {
		dst->value.s.x *= src->type == 1 ? src->value.s.x : (cs_int16)src->value.f.x;
		dst->value.s.y *= src->type == 1 ? src->value.s.y : (cs_int16)src->value.f.y;
		dst->value.s.z *= src->type == 1 ? src->value.s.z : (cs_int16)src->value.f.z;
	}

	lua_pushvalue(L, 1);
	return 1;
}

static int vec_div(lua_State *L) {
	LuaVector *dst = lua_checkvector(L, 1);
	LuaVector *src = lua_checkvector(L, 2);

	if(dst->type == 0) {
		dst->value.f.x /= src->type == 0 ? src->value.f.x : (cs_float)src->value.s.x;
		dst->value.f.y /= src->type == 0 ? src->value.f.y : (cs_float)src->value.s.y;
		dst->value.f.z /= src->type == 0 ? src->value.f.z : (cs_float)src->value.s.z;
	} else if(dst->type == 1) {
		dst->value.s.x /= src->type == 1 ? src->value.s.x : (cs_int16)src->value.f.x;
		dst->value.s.y /= src->type == 1 ? src->value.s.y : (cs_int16)src->value.f.y;
		dst->value.s.z /= src->type == 1 ? src->value.s.z : (cs_int16)src->value.f.z;
	}

	lua_pushvalue(L, 1);
	return 1;
}

static const luaL_Reg vectormeta[] = {
	{"iszero", vec_iszero},
	{"scale", vec_scale},

	{"setx", vec_setxvalue},
	{"sety", vec_setyvalue},
	{"setz", vec_setzvalue},
	{"set", vec_setvalue},

	{"getx", vec_getxvalue},
	{"gety", vec_getyvalue},
	{"getz", vec_getzvalue},
	{"get", vec_getvalue},

	{"__add", vec_add},
	{"__sub", vec_sub},
	{"__mul", vec_mul},
	{"__div", vec_div},
	{NULL, NULL}
};

static int vec_newfloat(lua_State *L) {
	LuaVector *vec = lua_newluavector(L);
	vec->type = 0;

	if(lua_gettop(L) > 2) {
		lua_getfield(L, -1, "set");
		lua_pushvalue(L, -2);
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_call(L, 4, 0);
	}

	return 1;
}

static int vec_newshort(lua_State *L) {
	LuaVector *vec = lua_newluavector(L);
	vec->type = 1;

	if(lua_gettop(L) > 2) {
		lua_getfield(L, -1, "set");
		lua_pushvalue(L, -2);
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_call(L, 4, 0);
	}

	return 1;
}

static const luaL_Reg vectorlib[] = {
	{"float", vec_newfloat},
	{"short", vec_newshort},
	{NULL, NULL}
};

int luaopen_vector(lua_State *L) {
	luaL_newmetatable(L, "Vector");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, vectormeta, 0);
	lua_pop(L, 1);

	luaL_register(L, "vector", vectorlib);
	return 1;
}
