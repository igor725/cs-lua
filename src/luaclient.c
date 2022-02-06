#include <core.h>
#include <client.h>
#include "luaplugin.h"
#include "luaclient.h"

Client *lua_checkclient(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, "__clientmt");
	luaL_argcheck(L, ud != NULL, idx, "'Client' expected");
	return (Client *)*ud;
}

void lua_pushclient(lua_State *L, Client *client) {
	if(!client) {
		lua_pushnil(L);
		return;
	}

	ClientID id = Client_GetID(client);
	lua_pushstring(L, "__clients");
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushnumber(L, id);
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushnumber(L, id);
		lua_newuserdata(L, sizeof(Client *));
		luaL_setmetatable(L, "__clientmt");
		lua_settable(L, -3);
		lua_pushnumber(L, id);
		lua_gettable(L, -2);
	}

	lua_remove(L, -2);
	void **ud = lua_touserdata(L, -1);
	*ud = client;
}

static int client_get(lua_State *L) {
	ClientID id = (ClientID)luaL_checknumber(L, 1);
	lua_pushclient(L, Client_GetByID(id));
	return 1;
}

static int client_getbcast(lua_State *L) {
	lua_pushclient(L, Broadcast);
	return 1;
}

static int client_getname(lua_State *L) {
	cs_str name = (cs_str)luaL_checkstring(L, 1);
	lua_pushclient(L, Client_GetByName(name));
	return 1;
}

static const luaL_Reg clientlib[] ={
	{"getbyid", client_get},
	{"getbyname", client_getname},
	{"getbroadcast", client_getbcast},
	{NULL, NULL}
};

static int meta_getid(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushnumber(L, Client_GetID(client));
	return 1;
}

static int meta_getname(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushstring(L, Client_GetName(client));
	return 1;
}

static const luaL_Reg clientmeta[] = {
	{"getid", meta_getid},
	{"getname", meta_getname},
	{NULL, NULL}
};

int luaopen_client(lua_State *L) {
	lua_pushstring(L, "__clients");
	lua_newtable(L);
	lua_settable(L, LUA_REGISTRYINDEX);

	luaL_newmetatable(L, "__clientmt");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, clientmeta, 0);
	lua_pop(L, 1);

	lua_addnumconst(L, MESSAGE_TYPE_CHAT);
	lua_addnumconst(L, MESSAGE_TYPE_STATUS1);
	lua_addnumconst(L, MESSAGE_TYPE_STATUS2);
	lua_addnumconst(L, MESSAGE_TYPE_STATUS3);
	lua_addnumconst(L, MESSAGE_TYPE_BRIGHT1);
	lua_addnumconst(L, MESSAGE_TYPE_BRIGHT2);
	lua_addnumconst(L, MESSAGE_TYPE_BRIGHT3);
	lua_addnumconst(L, MESSAGE_TYPE_ANNOUNCE);

	lua_addnumconst(L, PLAYER_STATE_INITIAL);
	lua_addnumconst(L, PLAYER_STATE_MOTD);
	lua_addnumconst(L, PLAYER_STATE_INGAME);

	luaL_register(L, "client", clientlib);
	return 1;
}
