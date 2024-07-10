#include <core.h>
#include <server.h>
#include <platform.h>
#include "luascript.h"
#include "luaconfig.h"

static int server_uptime(lua_State *L) {
	lua_pushinteger(L, Time_GetMSec() - Server_StartTime);
	return 1;
}

static int server_stop(lua_State *L) {
	(void)L;
	Server_Active = false;
	return 0;
}

static int server_info(lua_State *L) {
	ServerInfo si;
	if(Server_GetInfo(&si, sizeof(si))) {
		lua_createtable(L, 0, 3);
		lua_pushinteger(L, si.coreFlags);
		lua_setfield(L, -2, "flags");
		lua_pushstring(L, si.coreName);
		lua_setfield(L, -2, "software");
		lua_pushstring(L, si.coreGitTag);
		lua_setfield(L, -2, "tag");
	} else
		luaL_error(L, "Failed to fetch server info");

	return 1;
}

static const luaL_Reg serverlib[] = {
	{"uptime", server_uptime},
	{"stop", server_stop},
	{"info", server_info},

	{NULL, NULL}
};

int scr_libfunc(server)(lua_State *L) {
	scr_addintconst(L, SERVERINFO_FLAG_DEBUG);
	scr_addintconst(L, SERVERINFO_FLAG_WININET);
	scr_addintconst(L, SERVERINFO_FLAG_LIBCURL);
	scr_addintconst(L, SERVERINFO_FLAG_WINCRYPT);
	scr_addintconst(L, SERVERINFO_FLAG_LIBCRYPTO);

	luaL_newlib(L, serverlib);
	return 1;
}
