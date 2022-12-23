#include <core.h>
#include <str.h>
#include <vector.h>
#include <platform.h>
#include "luascript.h"
#include "luaangle.h"

cs_bool scr_isangle(scr_Context *L, int idx) {
	return scr_testmemtype(L, idx, CSSCRIPTS_MANGLE) != NULL;
}

Ang *scr_newangle(scr_Context *L) {
	Ang *ang = scr_allocmem(L, sizeof(Ang));
	Memory_Zero(ang, sizeof(Ang));
	scr_setmemtype(L, CSSCRIPTS_MANGLE);
	return ang;
}

Ang *scr_checkangle(scr_Context *L, int idx) {
	return scr_checkmemtype(L, idx, CSSCRIPTS_MANGLE);
}

Ang *scr_toangle(scr_Context *L, int idx) {
	return scr_testmemtype(L, idx, CSSCRIPTS_MANGLE);
}

static int ang_setvalue(scr_Context *L) {
	Ang *ang = scr_checkangle(L, 1);
	ang->yaw = (cs_float)scr_optnumber(L, 2, ang->yaw);
	ang->pitch = (cs_float)scr_optnumber(L, 3, ang->pitch);
	return 0;
}

static int ang_getvalue(scr_Context *L) {
	Ang *ang = scr_checkangle(L, 1);
	scr_pushnumber(L, (scr_Number)ang->yaw);
	scr_pushnumber(L, (scr_Number)ang->pitch);
	return 2;
}

static cs_bool getaxis(cs_str str, cs_char *ax) {
	if(String_CaselessCompare(str, "yaw")) {
		*ax = 'y';
		return true;
	} else if(String_CaselessCompare(str, "pitch")) {
		*ax = 'p';
		return true;
	}

	return false;
}

static int meta_index(scr_Context *L) {
	Ang *ang = scr_checkangle(L, 1);

	cs_char axis = 0;
	if(getaxis(scr_checkstring(L, 2), &axis)) {
		switch(axis) {
			case 'y': scr_pushnumber(L, (scr_Number)ang->yaw); break;
			case 'p': scr_pushnumber(L, (scr_Number)ang->pitch); break;
		}

		return 1;
	}

	scr_argerror(L, 2, CSSCRIPTS_MANGLE " axis expected");
	return 0;
}

static int meta_newindex(scr_Context *L) {
	Ang *ang = scr_checkangle(L, 1);

	cs_char axis = 0;
	if(getaxis(scr_checkstring(L, 2), &axis)) {
		switch(axis) {
			case 'y': ang->yaw = (cs_float)scr_checknumber(L, 3); break;
			case 'p': ang->pitch = (cs_float)scr_checknumber(L, 3); break;
		}

		return 0;
	}

	scr_argerror(L, 2, CSSCRIPTS_MANGLE " axis expected");
	return 0;
}

static int meta_call(scr_Context *L) {
	ang_setvalue(L);
	scr_stackpop(L, scr_stacktop(L) - 1);
	return 1;
}

static int meta_tostring(scr_Context *L) {
	Ang *ang = scr_checkangle(L, 1);
	scr_pushformatstring(L, "Angle(%f, %f)", ang->yaw, ang->pitch);
	return 1;
}

static const scr_RegFuncs anglemeta[] = {
	{"set", ang_setvalue},
	{"get", ang_getvalue},

	{"__call", meta_call},
	{"__index", meta_index},
	{"__newindex", meta_newindex},
	{"__tostring", meta_tostring},

	{NULL, NULL}
};

static int ang_new(scr_Context *L) {
	scr_newangle(L);

	if(scr_stacktop(L) > 1) {
		scr_getmetafield(L, -1, "set");
		scr_stackpush(L, -2);
		scr_stackpush(L, 1);
		scr_stackpush(L, 2);
		scr_unprotectedcall(L, 3, 0);
	}

	return 1;
}

int scr_libfunc(angle)(scr_Context *L) {
	scr_createtype(L, CSSCRIPTS_MANGLE, anglemeta);
	scr_pushnativefunc(L, ang_new);
	return 1;
}
