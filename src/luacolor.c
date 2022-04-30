#include <core.h>
#include <platform.h>
#include <types/cpe.h>
#include "luascript.h"
#include "luacolor.h"

cs_bool lua_iscolor(lua_State *L, int idx) {
	return luaL_testudata(L, idx, CSLUA_MCOLOR) != NULL;
}

LuaColor *lua_newcolor(lua_State *L) {
	LuaColor *col = lua_newuserdata(L, sizeof(LuaColor));
	Memory_Zero(&col->value, sizeof(col->value));
	luaL_setmetatable(L, CSLUA_MCOLOR);
	col->value.c4.a = 0xFF;
	return col;
}

LuaColor *lua_tocolor(lua_State *L, int idx) {
	return (LuaColor *)luaL_testudata(L, idx, CSLUA_MCOLOR);
}

LuaColor *lua_checkcolor(lua_State *L, int idx) {
	return (LuaColor *)luaL_checkudata(L, idx, CSLUA_MCOLOR);
}

Color3 *lua_tocolor3(lua_State *L, int idx) {
	LuaColor *col = lua_tocolor(L, idx);
	return col ? &col->value.c3 : NULL;
}

Color3 *lua_checkcolor3(lua_State *L, int idx) {
	LuaColor *col = lua_checkcolor(L, idx);
	luaL_argcheck(L, !col->hasAlpha, idx, "'Color3' expected");
	return &col->value.c3;
}

Color4 *lua_tocolor4(lua_State *L, int idx) {
	LuaColor *col = lua_tocolor(L, idx);
	return col ? &col->value.c4 : NULL;
}

Color4 *lua_checkcolor4(lua_State *L, int idx) {
	LuaColor *col = lua_checkcolor(L, idx);
	luaL_argcheck(L, col->hasAlpha, idx, "'Color4' expected");
	return &col->value.c4;
}

static int col_setvalue(lua_State *L) {
	LuaColor *col = lua_checkcolor(L, 1);

	col->value.c3.r = (cs_int16)luaL_checkinteger(L, 2);
	col->value.c3.g = (cs_int16)luaL_checkinteger(L, 3);
	col->value.c3.b = (cs_int16)luaL_checkinteger(L, 4);
	if(col->hasAlpha) col->value.c4.a = (cs_int16)luaL_checkinteger(L, 5);

	return 0;
}

static int col_getvalue(lua_State *L) {
	LuaColor *col = lua_checkcolor(L, 1);

	lua_pushinteger(L, (lua_Integer)col->value.c3.r);
	lua_pushinteger(L, (lua_Integer)col->value.c3.g);
	lua_pushinteger(L, (lua_Integer)col->value.c3.b);
	if(col->hasAlpha) {
		lua_pushinteger(L, (lua_Integer)col->value.c4.a);
		return 4;
	}

	return 3;
}

static cs_bool getclr(cs_str str, cs_char *cl) {
	if(*(str + 1) != '\0') return false;

	switch(*str) {
		case 'r': case 'R':
			*cl = 'r'; break;
		case 'g': case 'G':
			*cl = 'g'; break;
		case 'b': case 'B':
			*cl = 'b'; break;
		case 'a': case 'A':
			*cl = 'a'; break;
		default: return false;
	}

	return true;
}

static int meta_index(lua_State *L) {
	LuaColor *col = lua_checkcolor(L, 1);
	cs_str field = luaL_checkstring(L, 2);

	cs_char cli = 0;
	if(getclr(field, &cli)) {
		switch(cli) {
			case 'r':
				lua_pushinteger(L, (lua_Integer)col->value.c3.r);
				return 1;
			case 'g':
				lua_pushinteger(L, (lua_Integer)col->value.c3.g);
				return 1;
			case 'b':
				lua_pushinteger(L, (lua_Integer)col->value.c3.b);
				return 1;
			case 'a':
				lua_pushinteger(L, (lua_Integer)col->value.c4.a);
				return 1;
		}
	}

	luaL_getmetafield(L, 1, field);
	return 1;
}

static int meta_newindex(lua_State *L) {
	LuaColor *col = lua_checkcolor(L, 1);
	cs_str field = luaL_checkstring(L, 2);

	cs_char cli = 0;
	if(getclr(field, &cli)) {
		switch(cli) {
			case 'r':
				col->value.c3.r = (cs_int16)luaL_checkinteger(L, 3);
				return 1;
			case 'g':
				col->value.c3.g = (cs_int16)luaL_checkinteger(L, 3);
				return 1;
			case 'b':
				col->value.c3.b = (cs_int16)luaL_checkinteger(L, 3);
				return 1;
			case 'a':
				if(col->hasAlpha)
					col->value.c4.a = (cs_int16)luaL_checkinteger(L, 3);
				return 1;
		}

		return 0;
	}

	luaL_argerror(L, 2, "Primary color expected");
	return 0;
}

static int meta_tostring(lua_State *L) {
	LuaColor *col = lua_checkcolor(L, 1);
	lua_pushfstring(L, "Color(%d, %d, %d, %d)",
		col->value.c4.r, col->value.c4.g,
		col->value.c4.b, col->value.c4.a
	);
	return 1;
}

static int meta_eq(lua_State *L) {
	LuaColor *col1 = lua_checkcolor(L, 1);
	LuaColor *col2 = lua_checkcolor(L, 2);
	lua_pushboolean(L,
		col1->value.c3.r == col2->value.c3.r &&
		col1->value.c3.g == col2->value.c3.g &&
		col1->value.c3.b == col2->value.c3.b &&
		col1->value.c4.a == col2->value.c4.a
	);
	return 1;
}

static const luaL_Reg colormeta[] = {
	{"set", col_setvalue},
	{"get", col_getvalue},

	{"__newindex", meta_newindex},
	{"__tostring", meta_tostring},
	{"__index", meta_index},
	{"__eq", meta_eq},

	{NULL, NULL}
};

static int col_c3(lua_State *L) {
	LuaColor *col = lua_newcolor(L);
	col->hasAlpha = false;

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

static int col_c4(lua_State *L) {
	LuaColor *col = lua_newcolor(L);
	col->hasAlpha = true;

	if(lua_gettop(L) > 2) {
		lua_getfield(L, -1, "set");
		lua_pushvalue(L, -2);
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_pushvalue(L, 4);
		lua_call(L, 5, 0);
	}

	return 1;
}

static const luaL_Reg colorlib[] = {
	{"c3", col_c3},
	{"c4", col_c4},

	{NULL, NULL}
};

int luaopen_color(lua_State *L) {
	lua_indexedmeta(L, CSLUA_MCOLOR, colormeta);
	lua_pop(L, 1);

	luaL_newlib(L, colorlib);
	return 1;
}
