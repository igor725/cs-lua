#include <core.h>
#include <server.h>
#include <platform.h>
#include "luascript.h"
#include "luaconfig.h"

static int server_uptime(scr_Context *L) {
	scr_pushinteger(L, Time_GetMSec() - Server_StartTime);
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
		scr_newntable(L, 0, 3);
		scr_pushinteger(L, si.coreFlags);
		scr_settabfield(L, -2, "flags");
		scr_pushstring(L, si.coreName);
		scr_settabfield(L, -2, "software");
		scr_pushstring(L, si.coreGitTag);
		scr_settabfield(L, -2, "tag");
	} else
		scr_fmterror(L, "Failed to fetch server info");

	return 1;
}

static const scr_RegFuncs serverlib[] = {
	{"uptime", server_uptime},
	{"stop", server_stop},
	{"info", server_info},

	{NULL, NULL}
};

int scr_libfunc(server)(scr_Context *L) {
	scr_addintconst(L, SERVERINFO_FLAG_DEBUG);
	scr_addintconst(L, SERVERINFO_FLAG_WININET);
	scr_addintconst(L, SERVERINFO_FLAG_LIBCURL);
	scr_addintconst(L, SERVERINFO_FLAG_WINCRYPT);
	scr_addintconst(L, SERVERINFO_FLAG_LIBCRYPTO);

	scr_newlib(L, serverlib);
	return 1;
}
