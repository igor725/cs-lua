#include <core.h>
#include <command.h>
#include "luascript.h"
#include "luaclient.h"

static const char *errors[] = {
	// Lua returns
	"Command with the same name already exists",
	"Command with this name is not registred",
	"This command was not registred by this lua_State",
	"Command name cannot be empty",
	"Command description cannot be empty",

	// Command execution errors
	"&cLua execution error: %s",
	"function was not found in the registry table",
	
	// Server errors??
	"Command registration failed"
};

static Command *getstatecommand(lua_State *L, int idx) {
	Command *cmd = Command_GetByName(luaL_checkstring(L, idx));
	luaL_argcheck(L, cmd != NULL, idx, errors[1]);
	luaL_argcheck(L, Command_GetUserData(cmd) == lua_getscript(L), idx, errors[2]);
	return cmd;
}

COMMAND_FUNC(luacmd) {
	cs_bool succ = false;
	cs_str output = NULL;
	Command *cmd = ccdata->command;
	LuaScript *script = Command_GetUserData(cmd);

	LuaScript_Lock(script);
	if(LuaScript_RegistryLookup(script, CSLUA_RCMDS, Command_GetName(cmd))) {
		lua_pushclient(script->L, ccdata->caller);
		lua_pushstring(script->L, ccdata->args);
		succ = lua_pcall(script->L, 2, 1, 0) == 0;
		if(lua_isstring(script->L, -1))
			output = luaL_checkstring(script->L, -1);
		else if(!lua_isnoneornil(script->L, -1))
			output = "&e[non-string value]";
		lua_pop(script->L, 1);
	} else output = errors[6];
	LuaScript_Unlock(script);

	if(output) {
		if(succ) COMMAND_PRINT(output);
		COMMAND_PRINTF(errors[5], output);
	}

	return false;
}

static int cmd_add(lua_State *L) {
	cs_str name = luaL_checkstring(L, 1);
	luaL_argcheck(L, String_Length(name) > 0, 1, errors[3]);
	cs_str descr = luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(descr) > 0, 2, errors[4]);
	cs_byte flags = (cs_byte)luaL_checkinteger(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);

	if(Command_GetByName(name))
		luaL_error(L, errors[0]);

	Command *cmd = Command_Register(name, descr, svcmd_luacmd, flags);
	if(!cmd) luaL_error(L, errors[7]);
	Command_SetUserData(cmd, lua_getscript(L));
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCMDS);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 4);
	lua_settable(L, -3);

	return 0;
}

static int cmd_remove(lua_State *L) {
	Command *cmd = getstatecommand(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCMDS);
	lua_pushnil(L);
	lua_setfield(L, -2, Command_GetName(cmd));
	Command_Unregister(cmd);
	return 0;
}

static int cmd_setalias(lua_State *L) {
	Command *cmd = getstatecommand(L, 1);
	cs_str alias = luaL_checkstring(L, 2);
	lua_pushboolean(L, Command_SetAlias(cmd, alias));
	return 1;
}

static const luaL_Reg cmdlib[] ={
	{"add", cmd_add},
	{"setalias", cmd_setalias},
	{"remove", cmd_remove},

	{NULL, NULL}
};

int luaopen_command(lua_State *L) {
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, CSLUA_RCMDS);

	lua_addintconst(L, CMDF_NONE);
	lua_addintconst(L, CMDF_OP);
	lua_addintconst(L, CMDF_CLIENT);

	luaL_newlib(L, cmdlib);
	return 1;
}
