#include <log.h>
#include "lualog.h"
#include "luascript.h"

static void pushfmt(scr_Context *L) {
	int count = scr_stacktop(L);
	if(count > 0) {
		lua_getglobal(L, LUA_STRLIBNAME);
		scr_gettabfield(L, -1, "format");
		scr_stackrem(L, -2);
#		if LUA_VERSION_NUM < 502
			lua_getglobal(L, "tostring");
			scr_stackpush(L, 1);
			scr_unprotectedcall(L, 1, 1);
#		else
			luaL_tolstring(L, 1, NULL);
#		endif
		for(int i = 2; i <= count; i++)
			scr_stackpush(L, i);
		scr_unprotectedcall(L, count, 1);
	} else scr_pushstring(L, "nil");
}

static int log_info(scr_Context *L) {
	pushfmt(L);
	Log_Info("%s", scr_tostring(L, -1));
	return 0;
}

static int log_warn(scr_Context *L) {
	pushfmt(L);
	Log_Warn("%s", scr_tostring(L, -1));
	return 0;
}

static int log_error(scr_Context *L) {
	pushfmt(L);
	Log_Error("%s", scr_tostring(L, -1));
	return 0;
}

static int log_debug(scr_Context *L) {
	pushfmt(L);
	Log_Debug("%s", scr_tostring(L, -1));
	return 0;
}

static int log_print(scr_Context *L) {
	int count = scr_stacktop(L);
	if(count < 1) return 0;

#	if LUA_VERSION_NUM < 502
		lua_getglobal(L, "tostring");
#	endif

	for(int i = 1; i <= count; i++) {
#		if LUA_VERSION_NUM < 502
			scr_stackpush(L, -i);
			scr_stackpush(L, i);
			scr_unprotectedcall(L, 1, 1);
#		else
			luaL_tolstring(L, i, NULL);
#		endif
		scr_pushstring(L, " ");
		lua_concat(L, 2);
	}

	lua_concat(L, count);
	Log_Info("%s", scr_tostring(L, -1));
	return 0;
}

static const scr_RegFuncs loglib[] = {
	{"info", log_info},
	{"warn", log_warn},
	{"error", log_error},
	{"debug", log_debug},

	{NULL, NULL}
};

int scr_libfunc(log)(scr_Context *L) {
	scr_pushnativefunc(L, log_print);
	lua_setglobal(L, "print");

	scr_newlib(L, loglib);
	return 1;
}
