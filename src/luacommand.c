#include <core.h>
#include <client.h>
#include <command.h>
#include "luascript.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luaworld.h"

static const char *errors[] = {
	// Lua returns
	"Command already exists",
	"Command with this name is not registred",
	"This command was not registred by this lua_State",
	"Command name cannot be empty",
	"Command description cannot be empty",

	// Command execution errors
	"&cLua function was not found in the registry table",
	"&cLua execution error"
};

COMMAND_FUNC(luacmd) {
	cs_str output = NULL;
	Command *cmd = ccdata->command;
	LuaScript *script = Command_GetUserData(cmd);

	LuaScript_Lock(script);
	if(LuaScript_RegistryLookup(script, CSLUA_RCMDS, Command_GetName(cmd))) {
		lua_pushclient(script->L, ccdata->caller);
		lua_pushstring(script->L, ccdata->args);
		if(LuaScript_Call(script, 2, 1)) {
			if(lua_isstring(script->L, -1))
				output = luaL_checkstring(script->L, -1);
			lua_pop(script->L, 1);
		} else output = errors[6];
	} else output = errors[5];
	LuaScript_Unlock(script);

	if(output)
		COMMAND_PRINT(output);

	return false;
}

static int cmd_add(lua_State *L) {
	cs_str name = luaL_checkstring(L, 1);
	luaL_argcheck(L, String_Length(name) > 0, 1, errors[3]);
	cs_str descr = luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(descr) > 0, 2, errors[4]);
	cs_byte flags = (cs_byte)luaL_checkinteger(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);

	if(Command_GetByName(name)) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[0]);
		return 2;
	}

	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCMDS);
	lua_getfield(L, -1, name);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		Command *cmd = Command_Register(name, descr, svcmd_luacmd, flags);
		if(cmd) {
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 4);
			lua_settable(L, -3);
			cmd->data = lua_getscript(L);
		}
		lua_pop(L, 1);
		lua_pushboolean(L, cmd != NULL);
	} else {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[0]);
		return 2;
	}

	return 1;
}

static int cmd_remove(lua_State *L) {
	cs_str name = luaL_checkstring(L, 1);
	Command *cmd = Command_GetByName(name);

	if(!cmd) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[1]);
	} else if(Command_GetUserData(cmd) != lua_getscript(L)) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[2]);
	} else {
		Command_Unregister(cmd);
		lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCMDS);
		lua_pushnil(L);
		lua_setfield(L, -2, name);
		lua_pop(L, 1);
		lua_pushboolean(L, 1);
		lua_pushnil(L);
	}

	return 2;
}

static int cmd_setalias(lua_State *L) {
	cs_str name = luaL_checkstring(L, 1);
	cs_str alias = luaL_checkstring(L, 2);
	Command *cmd = Command_GetByName(name);

	if(!cmd) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[1]);
	} else if(Command_GetUserData(cmd) != lua_getscript(L)) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[2]);
	} else {
		Command_SetAlias(cmd, alias);
		lua_pushboolean(L, 1);
		lua_pushnil(L);
	}

	return 2;
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

	luaL_register(L, luaL_checkstring(L, 1), cmdlib);
	return 1;
}
