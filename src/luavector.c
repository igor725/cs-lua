#include <core.h>
#include <csmath.h>
#include <vector.h>
#include <platform.h>
#include "luascript.h"
#include "luavector.h"

cs_bool scr_isvector(lua_State *L, int idx) {
	return luaL_testudata(L, idx, CSSCRIPTS_MVECTOR) != NULL;
}

LuaVector *scr_newvector(lua_State *L) {
	LuaVector *vec = lua_newuserdata(L, sizeof(LuaVector));
	Memory_Zero(&vec->value, sizeof(vec->value));
	luaL_setmetatable(L, CSSCRIPTS_MVECTOR);
	vec->type = LUAVECTOR_TFLOAT;
	return vec;
}

LuaVector *scr_checkvector(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, CSSCRIPTS_MVECTOR);
}

LuaVector *scr_tovector(lua_State *L, int idx) {
	return luaL_testudata(L, idx, CSSCRIPTS_MVECTOR);
}

Vec *scr_checkfloatvector(lua_State *L, int idx) {
	LuaVector *vec = scr_checkvector(L, idx);
	luaL_argcheck(L, vec->type == LUAVECTOR_TFLOAT, idx, "'FloatVector' expected");
	return &vec->value.f;
}

Vec *scr_tofloatvector(lua_State *L, int idx) {
	LuaVector *vec = scr_tovector(L, idx);
	if(!vec || vec->type != LUAVECTOR_TFLOAT)
		return NULL;
	return &vec->value.f;
}

SVec *scr_checkshortvector(lua_State *L, int idx) {
	LuaVector *vec = scr_checkvector(L, idx);
	luaL_argcheck(L, vec->type == LUAVECTOR_TSHORT, idx, "'ShortVector' expected");
	return &vec->value.s;
}

SVec *scr_toshortvector(lua_State *L, int idx) {
	LuaVector *vec = scr_tovector(L, idx);
	if(!vec || vec->type != LUAVECTOR_TSHORT)
		return NULL;
	return &vec->value.s;
}

static int vec_iszero(lua_State *L) {
	LuaVector *vec = scr_checkvector(L, 1);

	if(vec->type == LUAVECTOR_TFLOAT)
		lua_pushboolean(L, Vec_IsZero(vec->value.f));
	else if(vec->type == LUAVECTOR_TSHORT)
		lua_pushboolean(L, Vec_IsZero(vec->value.s));

	return 1;
}

static int vec_scale(lua_State *L) {
	LuaVector *vec = scr_checkvector(L, 1);

	if(vec->type == LUAVECTOR_TFLOAT) {
		Vec_Scale(vec->value.f, (cs_float)luaL_checknumber(L, 2));
	} else if(vec->type == LUAVECTOR_TSHORT) {
		Vec_Scale(vec->value.s, (cs_int16)luaL_checkinteger(L, 2));
	}

	return 0;
}

static INL cs_float magnitude(Vec *src) {
	return Math_Sqrt(
		src->x * src->x +
		src->y * src->y +
		src->z * src->z
	);
}

static int vec_cross(lua_State *L) {
	LuaVector *dst = scr_checkvector(L, 1);

	if(dst->type == LUAVECTOR_TFLOAT) {
		Vec *src1 = scr_checkfloatvector(L, 2),
		*src2 = scr_checkfloatvector(L, 3);
		Vec_Cross(dst->value.f, *src1, *src2);
	} else if(dst->type == LUAVECTOR_TSHORT) {
		SVec *src1 = scr_checkshortvector(L, 2),
		*src2 = scr_checkshortvector(L, 3);
		Vec_Cross(dst->value.s, *src1, *src2);
	}

	lua_pop(L, lua_gettop(L) - 1);
	return 1;
}

static int vec_min(lua_State *L) {
	LuaVector *dst = scr_checkvector(L, 1);

	if(dst->type == LUAVECTOR_TFLOAT) {
		Vec *v1 = scr_checkfloatvector(L, 2),
		*v2 = scr_checkfloatvector(L, 3);
		Vec_Min(dst->value.f, *v1, *v2);
	} else if(dst->type == LUAVECTOR_TSHORT) {
		SVec *v1 = scr_checkshortvector(L, 2),
		*v2 = scr_checkshortvector(L, 3);
		Vec_Min(dst->value.s, *v1, *v2);
	}

	lua_pop(L, lua_gettop(L) - 1);
	return 1;
}

static int vec_max(lua_State *L) {
	LuaVector *dst = scr_checkvector(L, 1);

	if(dst->type == LUAVECTOR_TFLOAT) {
		Vec *v1 = scr_checkfloatvector(L, 2),
		*v2 = scr_checkfloatvector(L, 3);
		Vec_Max(dst->value.f, *v1, *v2);
	} else if(dst->type == LUAVECTOR_TSHORT) {
		SVec *v1 = scr_checkshortvector(L, 2),
		*v2 = scr_checkshortvector(L, 3);
		Vec_Max(dst->value.s, *v1, *v2);
	}

	lua_pop(L, lua_gettop(L) - 1);
	return 1;
}

static int vec_toshort(lua_State *L) {
	LuaVector *src = scr_checkvector(L, 1);
	LuaVector *dst = scr_newvector(L);
	dst->type = LUAVECTOR_TSHORT;

	if(src->type == LUAVECTOR_TFLOAT) {
		dst->value.s.x = (cs_int16)src->value.f.x;
		dst->value.s.y = (cs_int16)src->value.f.y;
		dst->value.s.z = (cs_int16)src->value.f.z;
	} else dst->value = src->value;

	return 1;
}

static int vec_tofloat(lua_State *L) {
	LuaVector *src = scr_checkvector(L, 1);
	LuaVector *dst = scr_newvector(L);
	dst->type = LUAVECTOR_TFLOAT;

	if(src->type == LUAVECTOR_TSHORT) {
		dst->value.f.x = (cs_float)src->value.s.x;
		dst->value.f.y = (cs_float)src->value.s.y;
		dst->value.f.z = (cs_float)src->value.s.z;
	} else dst->value = src->value;

	return 1;
}

static int vec_normalized(lua_State *L) {
	Vec *dst, *src = scr_checkfloatvector(L, 1);

	if(scr_isvector(L, 2))
		dst = scr_checkfloatvector(L, 2);
	else
		dst = &scr_newvector(L)->value.f;

	cs_float m = magnitude(src);

	*dst = *src;
	dst->x /= m, dst->y /= m, dst->z /= m;

	return 1;
}

static int vec_magnitude(lua_State *L) {
	lua_pushnumber(L, (lua_Number)magnitude(
		scr_checkfloatvector(L, 1)
	));
	return 1;
}

static int vec_setvalue(lua_State *L) {
	LuaVector *vec = scr_checkvector(L, 1);

	if(vec->type == LUAVECTOR_TFLOAT) {
		vec->value.f.x = (cs_float)luaL_optnumber(L, 2, vec->value.f.x);
		vec->value.f.y = (cs_float)luaL_optnumber(L, 3, vec->value.f.y);
		vec->value.f.z = (cs_float)luaL_optnumber(L, 4, vec->value.f.z);
	} else if(vec->type == LUAVECTOR_TSHORT) {
		vec->value.s.x = (cs_int16)luaL_optinteger(L, 2, vec->value.s.x);
		vec->value.s.y = (cs_int16)luaL_optinteger(L, 3, vec->value.s.y);
		vec->value.s.z = (cs_int16)luaL_optinteger(L, 4, vec->value.s.z);
	}

	return 0;
}

static int vec_getvalue(lua_State *L) {
	LuaVector *vec = scr_checkvector(L, 1);

	if(vec->type == LUAVECTOR_TFLOAT) {
		lua_pushnumber(L, (lua_Number)vec->value.f.x);
		lua_pushnumber(L, (lua_Number)vec->value.f.y);
		lua_pushnumber(L, (lua_Number)vec->value.f.z);
	} else if(vec->type == LUAVECTOR_TSHORT) {
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

static int meta_tostring(lua_State *L) {
	LuaVector *vec = scr_checkvector(L, 1);

	switch(vec->type) {
		case LUAVECTOR_TFLOAT:
			lua_pushfstring(L, "FloatVector(%f, %f, %f)",
				vec->value.f.x, vec->value.f.y, vec->value.f.z
			);
			break;
		case LUAVECTOR_TSHORT:
			lua_pushfstring(L, "ShortVector(%d, %d, %d)",
				vec->value.s.x, vec->value.s.y, vec->value.s.z
			);
			break;

		default:
			lua_pushfstring(L, "UnknownVector(%p)", vec);
			break;
	}

	return 1;
}

static int meta_index(lua_State *L) {
	LuaVector *vec = scr_checkvector(L, 1);
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
	LuaVector *vec = scr_checkvector(L, 1);

	cs_char ax = 0;
	if(getaxis(luaL_checkstring(L, 2), &ax)) {
		switch(ax) {
			case 'x':
				if(vec->type == LUAVECTOR_TFLOAT) vec->value.f.x = (cs_float)luaL_checknumber(L, 3);
				else vec->value.s.x = (cs_int16)luaL_checkinteger(L, 3);
				break;
			case 'y':
				if(vec->type == LUAVECTOR_TFLOAT) vec->value.f.y = (cs_float)luaL_checknumber(L, 3);
				else vec->value.s.y = (cs_int16)luaL_checkinteger(L, 3);
				break;
			case 'z':
				if(vec->type == LUAVECTOR_TFLOAT) vec->value.f.z = (cs_float)luaL_checknumber(L, 3);
				else vec->value.s.z = (cs_int16)luaL_checkinteger(L, 3);
				break;
		}

		return 0;
	}

	luaL_argerror(L, 2, CSSCRIPTS_MVECTOR " axis expected");
	return 0;
}

static int meta_call(lua_State *L) {
	vec_setvalue(L);
	lua_pop(L, lua_gettop(L) - 1);
	return 1;
}

static int meta_add(lua_State *L) {
	LuaVector *src1 = scr_checkvector(L, 1);
	LuaVector *src2 = scr_checkvector(L, 2);
	luaL_argcheck(L, src1->type == src2->type, 2, CSSCRIPTS_MVECTOR " types mismatch");

	LuaVector *dst = scr_newvector(L);
	dst->type = src1->type;

	if(dst->type == 0)
		Vec_Add(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == 1)
		Vec_Add(dst->value.s, src1->value.s, src2->value.s);

	return 1;
}

static int meta_sub(lua_State *L) {
	LuaVector *src1 = scr_checkvector(L, 1);
	LuaVector *src2 = scr_checkvector(L, 2);
	luaL_argcheck(L, src1->type == src2->type, 2, CSSCRIPTS_MVECTOR " types mismatch");

	LuaVector *dst = scr_newvector(L);
	dst->type = src1->type;

	if(dst->type == LUAVECTOR_TFLOAT)
		Vec_Sub(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == LUAVECTOR_TSHORT)
		Vec_Sub(dst->value.s, src1->value.s, src2->value.s);

	return 1;
}

static int meta_mul(lua_State *L) {
	LuaVector *src1 = scr_checkvector(L, 1);

	if(lua_isnumber(L, 2)) {
		LuaVector *dst = scr_newvector(L);
		dst->type = src1->type;

		if(dst->type == LUAVECTOR_TFLOAT) {
			dst->value.f = src1->value.f;
			Vec_Scale(dst->value.f, (cs_float)luaL_checknumber(L, 2));
		} else if(dst->type == LUAVECTOR_TSHORT) {
			dst->value.s = src1->value.s;
			Vec_Scale(dst->value.s, (cs_int16)luaL_checkinteger(L, 2));
		}

		return 1;
	}

	LuaVector *src2 = scr_checkvector(L, 2);
	luaL_argcheck(L, src1->type == src2->type, 2, CSSCRIPTS_MVECTOR " types mismatch");

	LuaVector *dst = scr_newvector(L);
	dst->type = src1->type;

	if(dst->type == LUAVECTOR_TFLOAT)
		Vec_Mul(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == LUAVECTOR_TSHORT)
		Vec_Mul(dst->value.s, src1->value.s, src2->value.s);

	return 1;
}

static int meta_div(lua_State *L) {
	LuaVector *src1 = scr_checkvector(L, 1);

	if(lua_isnumber(L, 2)) {
		LuaVector *dst = scr_newvector(L);
		dst->type = src1->type;

		if(dst->type == LUAVECTOR_TFLOAT) {
			dst->value.f = src1->value.f;
			Vec_DivN(dst->value.f, (cs_float)lua_tonumber(L, 2));
		} else if(dst->type == LUAVECTOR_TSHORT) {
			cs_int16 id = (cs_int16)lua_tointeger(L, 2);
			luaL_argcheck(L, id != 0, 2, "Integer division by zero");
			dst->value.s = src1->value.s;
			Vec_DivN(dst->value.s, id);
		}

		return 1;
	}

	LuaVector *src2 = scr_checkvector(L, 2);
	luaL_argcheck(L, src1->type == src2->type, 2, CSSCRIPTS_MVECTOR " types mismatch");

	LuaVector *dst = scr_newvector(L);
	dst->type = src1->type;

	if(dst->type == LUAVECTOR_TFLOAT)
		Vec_Div(dst->value.f, src1->value.f, src2->value.f);
	else if(dst->type == LUAVECTOR_TSHORT) {
		luaL_argcheck(L, !Vec_HaveZero(src2->value.s), 2, "Integer division by zero");
		Vec_Div(dst->value.s, src1->value.s, src2->value.s);
	}

	return 1;
}

static int meta_lt(lua_State *L) {
	LuaVector *vec1 = scr_checkvector(L, 1);
	LuaVector *vec2 = scr_checkvector(L, 2);
	luaL_argcheck(L, vec1->type == vec2->type, 2, CSSCRIPTS_MVECTOR " types mismatch");
	if(vec1->type == LUAVECTOR_TFLOAT)
		lua_pushboolean(L, vec1->value.f.x < vec2->value.f.x &&
			vec1->value.f.y < vec2->value.f.y &&
			vec1->value.f.z < vec2->value.f.z
		);
	else if(vec1->type == LUAVECTOR_TSHORT)
		lua_pushboolean(L, vec1->value.s.x < vec2->value.s.x &&
			vec1->value.s.y < vec2->value.s.y &&
			vec1->value.s.z < vec2->value.s.z
		);

	return 1;
}

static int meta_le(lua_State *L) {
	LuaVector *vec1 = scr_checkvector(L, 1);
	LuaVector *vec2 = scr_checkvector(L, 2);
	luaL_argcheck(L, vec1->type == vec2->type, 2, CSSCRIPTS_MVECTOR " types mismatch");
	if(vec1->type == LUAVECTOR_TFLOAT)
		lua_pushboolean(L, vec1->value.f.x <= vec2->value.f.x &&
			vec1->value.f.y <= vec2->value.f.y &&
			vec1->value.f.z <= vec2->value.f.z
		);
	else if(vec1->type == LUAVECTOR_TSHORT)
		lua_pushboolean(L, vec1->value.s.x <= vec2->value.s.x &&
			vec1->value.s.y <= vec2->value.s.y &&
			vec1->value.s.z <= vec2->value.s.z
		);

	return 1;
}

static int meta_eq(lua_State *L) {
	LuaVector *vec1 = scr_checkvector(L, 1);
	LuaVector *vec2 = scr_checkvector(L, 2);

	if(vec1->type == vec2->type) {
		if(vec1->type == LUAVECTOR_TSHORT)
			lua_pushboolean(L, SVec_Compare(&vec1->value.s, &vec2->value.s));
		else if(vec1->type == LUAVECTOR_TFLOAT)
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
	{"normalized", vec_normalized},
	{"magnitude", vec_magnitude},
	{"cross", vec_cross},

	{"min", vec_min},
	{"max", vec_max},

	{"toshort", vec_toshort},
	{"tofloat", vec_tofloat},

	{"set", vec_setvalue},
	{"get", vec_getvalue},

	{"__tostring", meta_tostring},
	{"__newindex", meta_newindex},
	{"__index", meta_index},
	{"__call", meta_call},
	{"__add", meta_add},
	{"__sub", meta_sub},
	{"__mul", meta_mul},
	{"__div", meta_div},
	{"__lt", meta_lt},
	{"__le", meta_le},
	{"__eq", meta_eq},

	{NULL, NULL}
};

static int vec_newfloat(lua_State *L) {
	LuaVector *vec = scr_newvector(L);
	vec->type = LUAVECTOR_TFLOAT;

	if(lua_gettop(L) > 2) {
		luaL_getmetafield(L, -1, "set");
		lua_pushvalue(L, -2);
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_call(L, 4, 0);
	}

	return 1;
}

static int vec_newshort(lua_State *L) {
	LuaVector *vec = scr_newvector(L);
	vec->type = LUAVECTOR_TSHORT;

	if(lua_gettop(L) > 2) {
		luaL_getmetafield(L, -1, "set");
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

int scr_libfunc(vector)(lua_State *L) {
	scr_createtype(L, CSSCRIPTS_MVECTOR, vectormeta);
	luaL_newlib(L, vectorlib);
	return 1;
}
