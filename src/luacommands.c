#include <core.h>
#include <client.h>
#include <command.h>
#include "luaplugin.h"
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
	"&cLua function not found in registry table",
	"&cLua execution error"
};

COMMAND_FUNC(luacmd) {
	cs_str output = NULL;
	Command *cmd = ccdata->command;
	LuaPlugin *plugin = Command_GetUserData(cmd);

	LuaPlugin_Lock(plugin);
	if(LuaPlugin_RegistryLookup(plugin, "__commands", Command_GetName(cmd))) {
		lua_pushclient(plugin->L, ccdata->caller);
		lua_pushstring(plugin->L, ccdata->args);
		if(LuaPlugin_Call(plugin, 2, 1)) {
			if(lua_isstring(plugin->L, -1))
				output = luaL_checkstring(plugin->L, -1);
			lua_pop(plugin->L, 1);
		} else output = errors[6];
	} else output = errors[5];
	LuaPlugin_Unlock(plugin);

	if(output) {
		COMMAND_PRINT(output);
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

	if(Command_GetByName(name)) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[0]);
		return 2;
	}

	lua_pushstring(L, "__commands");
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushvalue(L, 1);
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		Command *cmd = Command_Register(name, descr, svcmd_luacmd, flags);
		if(cmd) {
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 4);
			lua_settable(L, -3);
			cmd->data = lua_getplugin(L);
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
	} else if(Command_GetUserData(cmd) != lua_getplugin(L)) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, errors[2]);
	} else {
		Command_Unregister(cmd);
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
	} else if(Command_GetUserData(cmd) != lua_getplugin(L)) {
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
	lua_pushstring(L, "__commands");
	lua_newtable(L);
	lua_settable(L, LUA_REGISTRYINDEX);

	lua_addnumconst(L, CMDF_NONE);
	lua_addnumconst(L, CMDF_OP);
	lua_addnumconst(L, CMDF_CLIENT);

	luaL_register(L, "command", cmdlib);
	return 1;
}
