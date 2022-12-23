#include <core.h>
#include <platform.h>
#include <types/cpe.h>
#include "luascript.h"
#include "luacolor.h"

cs_bool scr_iscolor(scr_Context *L, int idx) {
	return scr_testmemtype(L, idx, CSSCRIPTS_MCOLOR) != NULL;
}

LuaColor *scr_newcolor(scr_Context *L) {
	LuaColor *col = scr_allocmem(L, sizeof(LuaColor));
	Memory_Zero(&col->value, sizeof(col->value));
	scr_setmemtype(L, CSSCRIPTS_MCOLOR);
	col->value.c4.a = 0xFF;
	return col;
}

LuaColor *scr_tocolor(scr_Context *L, int idx) {
	return scr_testmemtype(L, idx, CSSCRIPTS_MCOLOR);
}

LuaColor *scr_checkcolor(scr_Context *L, int idx) {
	return scr_checkmemtype(L, idx, CSSCRIPTS_MCOLOR);
}

Color3 *scr_tocolor3(scr_Context *L, int idx) {
	LuaColor *col = scr_tocolor(L, idx);
	return col ? &col->value.c3 : NULL;
}

Color3 *scr_checkcolor3(scr_Context *L, int idx) {
	LuaColor *col = scr_checkcolor(L, idx);
	scr_argassert(L, !col->hasAlpha, idx, "'Color3' expected");
	return &col->value.c3;
}

Color4 *scr_tocolor4(scr_Context *L, int idx) {
	LuaColor *col = scr_tocolor(L, idx);
	return col ? &col->value.c4 : NULL;
}

Color4 *scr_checkcolor4(scr_Context *L, int idx) {
	LuaColor *col = scr_checkcolor(L, idx);
	scr_argassert(L, col->hasAlpha, idx, "'Color4' expected");
	return &col->value.c4;
}

static int col_setvalue(scr_Context *L) {
	LuaColor *col = scr_checkcolor(L, 1);

	col->value.c3.r = (cs_int16)scr_checkinteger(L, 2);
	col->value.c3.g = (cs_int16)scr_checkinteger(L, 3);
	col->value.c3.b = (cs_int16)scr_checkinteger(L, 4);
	if(col->hasAlpha) col->value.c4.a = (cs_int16)scr_checkinteger(L, 5);

	return 0;
}

static int col_getvalue(scr_Context *L) {
	LuaColor *col = scr_checkcolor(L, 1);

	scr_pushinteger(L, (scr_Integer)col->value.c3.r);
	scr_pushinteger(L, (scr_Integer)col->value.c3.g);
	scr_pushinteger(L, (scr_Integer)col->value.c3.b);
	if(col->hasAlpha) {
		scr_pushinteger(L, (scr_Integer)col->value.c4.a);
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

static int meta_index(scr_Context *L) {
	LuaColor *col = scr_checkcolor(L, 1);
	cs_str field = scr_checkstring(L, 2);

	cs_char cli = 0;
	if(getclr(field, &cli)) {
		switch(cli) {
			case 'r':
				scr_pushinteger(L, (scr_Integer)col->value.c3.r);
				return 1;
			case 'g':
				scr_pushinteger(L, (scr_Integer)col->value.c3.g);
				return 1;
			case 'b':
				scr_pushinteger(L, (scr_Integer)col->value.c3.b);
				return 1;
			case 'a':
				scr_pushinteger(L, (scr_Integer)col->value.c4.a);
				return 1;
		}
	}

	scr_getmetafield(L, 1, field);
	return 1;
}

static int meta_newindex(scr_Context *L) {
	LuaColor *col = scr_checkcolor(L, 1);
	cs_str field = scr_checkstring(L, 2);

	cs_char cli = 0;
	if(getclr(field, &cli)) {
		switch(cli) {
			case 'r':
				col->value.c3.r = (cs_int16)scr_checkinteger(L, 3);
				return 1;
			case 'g':
				col->value.c3.g = (cs_int16)scr_checkinteger(L, 3);
				return 1;
			case 'b':
				col->value.c3.b = (cs_int16)scr_checkinteger(L, 3);
				return 1;
			case 'a':
				if(col->hasAlpha)
					col->value.c4.a = (cs_int16)scr_checkinteger(L, 3);
				return 1;
		}

		return 0;
	}

	scr_argerror(L, 2, "Primary color expected");
	return 0;
}

static int meta_tostring(scr_Context *L) {
	LuaColor *col = scr_checkcolor(L, 1);
	scr_pushformatstring(L, "Color(%d, %d, %d, %d)",
		col->value.c4.r, col->value.c4.g,
		col->value.c4.b, col->value.c4.a
	);
	return 1;
}

static int meta_eq(scr_Context *L) {
	LuaColor *col1 = scr_checkcolor(L, 1);
	LuaColor *col2 = scr_checkcolor(L, 2);
	scr_pushboolean(L,
		col1->value.c3.r == col2->value.c3.r &&
		col1->value.c3.g == col2->value.c3.g &&
		col1->value.c3.b == col2->value.c3.b &&
		col1->value.c4.a == col2->value.c4.a
	);
	return 1;
}

static const scr_RegFuncs colormeta[] = {
	{"set", col_setvalue},
	{"get", col_getvalue},

	{"__newindex", meta_newindex},
	{"__tostring", meta_tostring},
	{"__index", meta_index},
	{"__eq", meta_eq},

	{NULL, NULL}
};

static int col_c3(scr_Context *L) {
	LuaColor *col = scr_newcolor(L);
	col->hasAlpha = false;

	if(scr_stacktop(L) > 2) {
		scr_gettabfield(L, -1, "set");
		scr_stackpush(L, -2);
		scr_stackpush(L, 1);
		scr_stackpush(L, 2);
		scr_stackpush(L, 3);
		scr_unprotectedcall(L, 4, 0);
	}

	return 1;
}

static int col_c4(scr_Context *L) {
	LuaColor *col = scr_newcolor(L);
	col->hasAlpha = true;

	if(scr_stacktop(L) > 2) {
		scr_gettabfield(L, -1, "set");
		scr_stackpush(L, -2);
		scr_stackpush(L, 1);
		scr_stackpush(L, 2);
		scr_stackpush(L, 3);
		scr_stackpush(L, 4);
		scr_unprotectedcall(L, 5, 0);
	}

	return 1;
}

static const scr_RegFuncs colorlib[] = {
	{"c3", col_c3},
	{"c4", col_c4},

	{NULL, NULL}
};

int scr_libfunc(color)(scr_Context *L) {
	scr_createtype(L, CSSCRIPTS_MCOLOR, colormeta);
	scr_newlib(L, colorlib);
	return 1;
}
