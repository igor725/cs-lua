#include <core.h>
#include <vector.h>
#include <platform.h>
#include "luaplugin.h"
#include "luavector.h"

LuaVector *lua_newvector(lua_State *L) {
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

static cs_bool getaxis(cs_str str, cs_char *ax) {
	if(*(str + 1) != '\0') return false;

	switch(*str) {
		case 'x': case 'X':
			*ax = 'x'; break;
		case 'y': case 'Y':
			*ax = 'y'; break;
		case 'z': case 'Z':
			*ax = 'z'; break;
		default: return false;
	}

	return true;
}

static int meta_index(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);
	cs_str field = luaL_checkstring(L, 2);

	cs_char ax = 0;
	if(getaxis(field, &ax)) {
		switch(ax) {
			case 'x':
				if(vec->type == 0) lua_pushnumber(L, (lua_Number)vec->value.f.x);
				else lua_pushinteger(L, (lua_Integer)vec->value.s.x);
				break;
			case 'y':
				if(vec->type == 0) lua_pushnumber(L, (lua_Number)vec->value.f.y);
				else lua_pushinteger(L, (lua_Integer)vec->value.s.y);
				break;
			case 'z':
				if(vec->type == 0) lua_pushnumber(L, (lua_Number)vec->value.f.z);
				else lua_pushinteger(L, (lua_Integer)vec->value.s.z);
				break;
		}

		return 1;
	}

	luaL_getmetafield(L, 1, field);
	return 1;
}

static int meta_newindex(lua_State *L) {
	LuaVector *vec = lua_checkvector(L, 1);

	cs_char ax = 0;
	if(getaxis(luaL_checkstring(L, 2), &ax)) {
		switch(ax) {
			case 'x':
				if(vec->type == 0) vec->value.f.x = (cs_float)luaL_checknumber(L, 3);
				else vec->value.s.x = (cs_int16)luaL_checkinteger(L, 3);
				break;
			case 'y':
				if(vec->type == 0) vec->value.f.y = (cs_float)luaL_checknumber(L, 3);
				else vec->value.s.y = (cs_int16)luaL_checkinteger(L, 3);
				break;
			case 'z':
				if(vec->type == 0) vec->value.f.z = (cs_float)luaL_checknumber(L, 3);
				else vec->value.s.z = (cs_int16)luaL_checkinteger(L, 3);
				break;
		}

		return 0;
	}

	luaL_argerror(L, 2, "Vector axis expected");
	return 0;
}

static int meta_add(lua_State *L) {
	LuaVector *src1 = lua_checkvector(L, 1);
	LuaVector *src2 = lua_checkvector(L, 2);
	luaL_argcheck(L, src1->type != src2->type, 2, "Vector types mismatch");

	LuaVector *dst = lua_newvector(L);
	dst->type = src1->type;

	if(dst->type == 0)
		Vec_Add(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == 1)
		Vec_Add(dst->value.s, src1->value.s, src2->value.s);

	return 1;
}

static int meta_sub(lua_State *L) {
	LuaVector *src1 = lua_checkvector(L, 1);
	LuaVector *src2 = lua_checkvector(L, 2);
	luaL_argcheck(L, src1->type != src2->type, 2, "Vector types mismatch");

	LuaVector *dst = lua_newvector(L);
	dst->type = src1->type;

	if(dst->type == 0)
		Vec_Sub(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == 1)
		Vec_Sub(dst->value.s, src1->value.s, src2->value.s);

	return 1;
}

static int meta_mul(lua_State *L) {
	LuaVector *src1 = lua_checkvector(L, 1);
	LuaVector *src2 = lua_checkvector(L, 2);
	luaL_argcheck(L, src1->type != src2->type, 2, "Vector types mismatch");

	LuaVector *dst = lua_newvector(L);
	dst->type = src1->type;

	if(dst->type == 0)
		Vec_Mul(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == 1)
		Vec_Mul(dst->value.s, src1->value.s, src2->value.s);

	return 1;
}

static int meta_div(lua_State *L) {
	LuaVector *src1 = lua_checkvector(L, 1);
	LuaVector *src2 = lua_checkvector(L, 2);
	luaL_argcheck(L, src1->type != src2->type, 2, "Vector types mismatch");

	LuaVector *dst = lua_newvector(L);
	dst->type = src1->type;

	if(dst->type == 0)
		Vec_Div(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == 1)
		Vec_Div(dst->value.s, src1->value.s, src2->value.s);

	return 1;
}

static int meta_eq(lua_State *L) {
	LuaVector *vec1 = lua_checkvector(L, 1);
	LuaVector *vec2 = lua_checkvector(L, 2);

	if(vec1->type == vec2->type) {
		if(vec1->type == 1)
			lua_pushboolean(L, SVec_Compare(&vec1->value.s, &vec2->value.s));
		else if(vec1->type == 0)
			lua_pushboolean(L, Vec_Compare(&vec1->value.f, &vec2->value.f));
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	lua_pushboolean(L, 0);
	return 1;
}

static const luaL_Reg vectormeta[] = {
	{"iszero", vec_iszero},
	{"scale", vec_scale},

	{"set", vec_setvalue},
	{"get", vec_getvalue},

	{"__index", meta_index},
	{"__newindex", meta_newindex},
	{"__add", meta_add},
	{"__sub", meta_sub},
	{"__mul", meta_mul},
	{"__div", meta_div},
	{"__eq", meta_eq},

	{NULL, NULL}
};

static int vec_newfloat(lua_State *L) {
	LuaVector *vec = lua_newvector(L);
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
	LuaVector *vec = lua_newvector(L);
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
	luaL_setfuncs(L, vectormeta, 0);
	lua_pop(L, 1);

	luaL_register(L, luaL_checkstring(L, 1), vectorlib);
	return 1;
}
