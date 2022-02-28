#include <core.h>
#include <log.h>
#include "lualog.h"

static void pushfmt(lua_State *L) {
	int count = lua_gettop(L);
	if(count > 0) {
		lua_getglobal(L, LUA_STRLIBNAME);
		lua_getfield(L, -1, "format");
		lua_remove(L, -2);
#		if LUA_VERSION_NUM < 502
			lua_getglobal(L, "tostring");
			lua_pushvalue(L, 1);
			lua_call(L, 1, 1);
#		else
			luaL_tolstring(L, 1, NULL);
#		endif
		for(int i = 2; i <= count; i++)
			lua_pushvalue(L, i);
		lua_call(L, count, 1);
	} else lua_pushstring(L, "nil");
}

static int log_info(lua_State *L) {
	pushfmt(L);
	Log_Info("%s", lua_tostring(L, -1));
	return 0;
}

static int log_warn(lua_State *L) {
	pushfmt(L);
	Log_Warn("%s", lua_tostring(L, -1));
	return 0;
}

static int log_error(lua_State *L) {
	pushfmt(L);
	Log_Error("%s", lua_tostring(L, -1));
	return 0;
}

static int log_debug(lua_State *L) {
	pushfmt(L);
	Log_Debug("%s", lua_tostring(L, -1));
	return 0;
}

const luaL_Reg loglib[] = {
	{"info", log_info},
	{"warn", log_warn},
	{"error", log_error},
	{"debug", log_debug},
	{NULL, NULL}
};

int luaopen_log(lua_State *L) {
	luaL_register(L, luaL_checkstring(L, 1), loglib);
	return 1;
}
