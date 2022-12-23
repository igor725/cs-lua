#include <core.h>
#include <groups.h>
#include "luascript.h"
#include "luagroup.h"

static int group_add(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Groups_Create(
		scr_checkstring(L, 1),
		(cs_byte)scr_checkinteger(L, 2)
	));
	return 1;
}

static int group_remove(scr_Context *L) {
	scr_pushboolean(L, Groups_Remove(
		(cs_uintptr)scr_checkinteger(L, 1)
	));
	return 1;
}

static int group_getinfo(scr_Context *L) {
	CGroup *grp = Groups_GetByID((cs_uintptr)scr_checkinteger(L, 1));
	if(grp) {
		scr_pushinteger(L, (scr_Integer)grp->rank);
		scr_pushstring(L, grp->name);
		return 2;
	}

	scr_pushnull(L);
	return 1;
}

static const scr_RegFuncs grouplib[] = {
	{"add", group_add},
	{"remove", group_remove},
	{"getinfo", group_getinfo},

	{NULL, NULL}
};

int scr_libfunc(group)(scr_Context *L) {
	scr_addintconst(L, GROUPS_INVALID_ID);

	scr_newlib(L, grouplib);
	return 1;
}
