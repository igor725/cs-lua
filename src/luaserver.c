#include <core.h>
#include <server.h>
#include <platform.h>
#include "luascript.h"
#include "luaconfig.h"

static int server_uptime(scr_Context *L) {
	lua_pushinteger(L, Time_GetMSec() - Server_StartTime);
	return 1;
}

static int server_stop(scr_Context *L) {
	(void)L;
	Server_Active = false;
	return 0;
}

static int server_info(scr_Context *L) {
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

int luaopen_server(scr_Context *L) {
	lua_addintconst(L, SERVERINFO_FLAG_DEBUG);
	lua_addintconst(L, SERVERINFO_FLAG_WININET);
	lua_addintconst(L, SERVERINFO_FLAG_LIBCURL);
	lua_addintconst(L, SERVERINFO_FLAG_WINCRYPT);
	lua_addintconst(L, SERVERINFO_FLAG_LIBCRYPTO);

	luaL_newlib(L, serverlib);
	return 1;
}
