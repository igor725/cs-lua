#include <core.h>
#include <command.h>
#include "luascript.h"
#include "luaclient.h"

static const char *errors[] = {
	// Lua returns
	"Command with the same name already exists",
	"Command with this name is not registred",
	"This command was not registred by this scr_Context",
	"Command name cannot be empty",
	"Command description cannot be empty",

	// Command execution errors
	"&cLua execution error: %s",
	"function was not found in the registry table",

	// Server errors??
	"Command registration failed"
};

static Command *getstatecommand(scr_Context *L, int idx) {
	Command *cmd = Command_GetByName(scr_checkstring(L, idx));
	scr_argassert(L, cmd != NULL, idx, errors[1]);
	scr_argassert(L, Command_GetUserData(cmd) == Script_GetHandle(L), idx, errors[2]);
	return cmd;
}

COMMAND_FUNC(luacmd) {
	cs_bool succ = false;
	cs_str output = NULL;
	Command *cmd = ccdata->command;
	Script *script = Command_GetUserData(cmd);

	Script_Lock(script);
	if(Script_RegistryLookup(script, CSSCRIPTS_RCMDS, Command_GetName(cmd))) {
		scr_pushclient(script->L, ccdata->caller);
		scr_pushstring(script->L, ccdata->args);
		succ = scr_protectedcall(script->L, 2, 1, 0) == 0;
		if(scr_isstring(script->L, -1))
			output = scr_checkstring(script->L, -1);
		else if(!scr_isemptyornull(script->L, -1))
			output = "&e[non-string value]";
		scr_stackpop(script->L, 1);
	} else output = errors[6];
	Script_Unlock(script);

	if(output) {
		if(succ) COMMAND_PRINT(output);
		COMMAND_PRINTF(errors[5], output);
	}

	return false;
}

static int cmd_add(scr_Context *L) {
	cs_str name = scr_checkstring(L, 1);
	scr_argassert(L, String_Length(name) > 0, 1, errors[3]);
	cs_str descr = scr_checkstring(L, 2);
	scr_argassert(L, String_Length(descr) > 0, 2, errors[4]);
	cs_byte flags = (cs_byte)scr_checkinteger(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);

	if(Command_GetByName(name))
		scr_fmterror(L, errors[0]);

	Command *cmd = Command_Register(name, descr, svcmd_luacmd, flags);
	if(!cmd) scr_fmterror(L, errors[7]);
	Command_SetUserData(cmd, Script_GetHandle(L));
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCMDS);
	scr_stackpush(L, 1);
	scr_stackpush(L, 4);
	scr_settotable(L, -3);

	return 0;
}

static int cmd_remove(scr_Context *L) {
	Command *cmd = getstatecommand(L, 1);
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCMDS);
	scr_pushnull(L);
	scr_settabfield(L, -2, Command_GetName(cmd));
	Command_Unregister(cmd);
	return 0;
}

static int cmd_setalias(scr_Context *L) {
	Command *cmd = getstatecommand(L, 1);
	cs_str alias = scr_checkstring(L, 2);
	scr_pushboolean(L, Command_SetAlias(cmd, alias));
	return 1;
}

static const scr_RegFuncs cmdlib[] ={
	{"add", cmd_add},
	{"setalias", cmd_setalias},
	{"remove", cmd_remove},

	{NULL, NULL}
};

int scr_libfunc(command)(scr_Context *L) {
	scr_newtable(L);
	scr_settabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCMDS);

	scr_addintconst(L, CMDF_NONE);
	scr_addintconst(L, CMDF_OP);
	scr_addintconst(L, CMDF_CLIENT);

	scr_newlib(L, cmdlib);
	return 1;
}
