#include <core.h>
#include <str.h>
#include <client.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luaworld.h"
#include "luacolor.h"

Client *lua_checkclient(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, "Client");
	luaL_argcheck(L, *ud != NULL, idx, "Invalid client");
	return (Client *)*ud;
}

void lua_pushclient(lua_State *L, Client *client) {
	if(!client) {
		lua_pushnil(L);
		return;
	}

	ClientID id = Client_GetID(client);
	lua_getfield(L, LUA_REGISTRYINDEX, "__clients");
	lua_pushnumber(L, id);
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushnumber(L, id);
		lua_newuserdata(L, sizeof(Client *));
		luaL_setmetatable(L, "Client");
		lua_settable(L, -3);
		lua_pushnumber(L, id);
		lua_gettable(L, -2);
	}

	lua_remove(L, -2);
	void **ud = lua_touserdata(L, -1);
	*ud = client;
}

void lua_clearclient(lua_State *L, Client *client) {
	lua_getfield(L, LUA_REGISTRYINDEX, "__clients");
	lua_pushinteger(L, Client_GetID(client));
	lua_gettable(L, -2);
	if(!lua_isuserdata(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	void **ud = lua_touserdata(L, -1);
	*ud = NULL;
}

static int meta_getid(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushinteger(L, (lua_Integer)Client_GetID(client));
	return 1;
}

static int meta_getaddr(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_uint32 addr = Client_GetAddr(client);
	lua_pushfstring(L, "%d.%d.%d.%d",
		(addr >> 24) & 0xFF,
		(addr >> 16) & 0xFF,
		(addr >> 8) & 0xFF,
		(addr & 0xFF)
	);
	return 1;
}

static int meta_getaddrn(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushinteger(L, (lua_Integer)Client_GetAddr(client));
	return 1;
}

static int meta_getping(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushinteger(L, (lua_Integer)Client_GetPing(client));
	return 1;
}

static int meta_getname(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushstring(L, Client_GetName(client));
	return 1;
}

static int meta_getappname(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushstring(L, Client_GetAppName(client));
	return 1;
}

static int meta_getposition(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	Vec *vec = lua_checkfloatvector(L, 2);
	lua_pushboolean(L, Client_GetPosition(client, vec, NULL));
	return 1;
}

static int meta_getpositiona(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	LuaVector *vec = lua_newvector(L);
	if(Client_GetPosition(client, &vec->value.f, NULL))
		vec->type = 0;
	else {
		lua_pop(L, 1);
		lua_pushnil(L);
	}
	return 1;
}

static int meta_getrotation(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	Ang *ang = lua_checkangle(L, 2);
	lua_pushboolean(L, Client_GetPosition(client, NULL, ang));
	return 1;
}

static int meta_getrotationa(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	Ang *ang = lua_newangle(L);
	if(Client_GetPosition(client, NULL, ang))
		return 1;

	lua_pop(L, 1);
	lua_pushnil(L);
	return 1;
}

static int meta_getclickdist(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushinteger(L, (lua_Integer)Client_GetClickDistance(client));
	return 1;
}

static int meta_getmodel(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushinteger(L, (lua_Integer)Client_GetModel(client));
	return 1;
}

static int meta_getworld(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushworld(L, Client_GetWorld(client));
	return 1;
}

static int meta_setop(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_bool state = (cs_bool)lua_toboolean(L, 2);
	lua_pushboolean(L, Client_SetOP(client, state));
	return 1;
}

static int meta_setweather(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	EWeather weather = (EWeather)luaL_checkinteger(L, 2);
	lua_pushboolean(L, Client_SetWeather(client, weather));
	return 1;
}

static int meta_setclickdist(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_int16 dist = (cs_int16)luaL_checkinteger(L, 2);
	lua_pushboolean(L, Client_SetClickDistance(client, dist));
	return 1;
}

static int meta_setmodel(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	if(lua_isnumber(L, 2))
		lua_pushboolean(L, Client_SetModel(client, (cs_int16)luaL_checkinteger(L, 2)));
	else
		lua_pushboolean(L, Client_SetModelStr(client, luaL_checkstring(L, 2)));
	return 1;
}

static int meta_setheldblock(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	BlockID block = (BlockID)luaL_checkinteger(L, 2);
	cs_bool cc = (cs_bool)lua_toboolean(L, 3);
	lua_pushboolean(L, Client_SetHeldBlock(client, block, cc));
	return 1;
}

static int meta_setskin(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_str url = (cs_str)luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(url) < 65, 2, "URL is too long");
	lua_pushboolean(L, Client_SetSkin(client, url));
	return 1;
}

static int meta_setvelocity(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	Vec *vec = lua_checkfloatvector(L, 2);
	lua_pushboolean(L, Client_SetVelocity(client, vec, true));
	return 1;
}

static int meta_gotoworld(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	World *world = lua_checkworld(L, 2);
	lua_pushboolean(L, Client_ChangeWorld(client, world));
	return 1;
}

static int meta_reload(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	World *world = Client_GetWorld(client);
	lua_pushboolean(L, Client_ChangeWorld(client, world));
	return 1;
}

static int meta_islocal(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushboolean(L, Client_IsLocal(client));
	return 1;
}

static int meta_isfirstspawn(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushboolean(L, Client_IsFirstSpawn(client));
	return 1;
}

static int meta_isinstate(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	EPlayerState state = (EPlayerState)luaL_checkinteger(L, 2);
	lua_pushboolean(L, Client_CheckState(client, state));
	return 1;
}

static int meta_isop(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushboolean(L, Client_IsOP(client));
	return 1;
}

static int meta_makeselection(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_byte id = (cs_byte)luaL_checkinteger(L, 2);
	SVec *start = lua_checkshortvector(L, 3);
	SVec *end = lua_checkshortvector(L, 4);
	Color4 *col = lua_checkcolor4(L, 5);
	lua_pushboolean(L, Client_MakeSelection(client, id, start, end, col));
	return 1;
}

static int meta_removeselection(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_byte id = (cs_byte)luaL_checkinteger(L, 2);
	lua_pushboolean(L, Client_RemoveSelection(client, id));
	return 1;
}

static int meta_update(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushboolean(L, Client_Update(client));
	return 1;
}

static int meta_teleport(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	Vec *vec = lua_checkfloatvector(L, 2);
	Ang *ang = lua_checkangle(L, 3);
	lua_pushboolean(L, Client_TeleportTo(client, vec, ang));
	return 1;
}

static int meta_kick(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_str reason = luaL_checkstring(L, 2);
	Client_Kick(client, reason);
	return 0;
}

static int meta_chat(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	EMesgType type = MESSAGE_TYPE_CHAT;
	cs_str mesg = NULL;

	if(lua_gettop(L) == 2)
		mesg = luaL_checkstring(L, 2);
	else {
		type = (EMesgType)luaL_checkinteger(L, 2);
		mesg = luaL_checkstring(L, 3);
	}

	Client_Chat(client, type, mesg);
	return 0;
}

static const luaL_Reg clientmeta[] = {
	{"getid", meta_getid},
	{"getaddr", meta_getaddr},
	{"getaddrn", meta_getaddrn},
	{"getping", meta_getping},
	{"getname", meta_getname},
	{"getappname", meta_getappname},
	{"getposition", meta_getposition},
	{"getpositiona", meta_getpositiona},
	{"getrotation", meta_getrotation},
	{"getrotationa", meta_getrotationa},
	{"getclickdist", meta_getclickdist},
	{"getmodel", meta_getmodel},
	{"getworld", meta_getworld},

	{"setop", meta_setop},
	{"setweather", meta_setweather},
	{"setclickdist", meta_setclickdist},
	{"setmodel", meta_setmodel},
	{"setheldblock", meta_setheldblock},
	{"setskin", meta_setskin},
	{"setvelocity", meta_setvelocity},
	{"gotoworld", meta_gotoworld},
	{"reload", meta_reload},

	{"islocal", meta_islocal},
	{"isfirstspawn", meta_isfirstspawn},
	{"isinstate", meta_isinstate},
	{"isop", meta_isop},

	{"makesel", meta_makeselection},
	{"remsel", meta_removeselection},
	{"update", meta_update},
	{"teleport", meta_teleport},
	{"kick", meta_kick},
	{"chat", meta_chat},

	{NULL, NULL}
};

static int client_get(lua_State *L) {
	int count = lua_gettop(L);
	for(int i = 1; i <= count; i++) {
		ClientID id = (ClientID)luaL_checknumber(L, i);
		lua_pushclient(L, Client_GetByID(id));
	}
	return count;
}

static int client_getbcast(lua_State *L) {
	lua_pushclient(L, Broadcast);
	return 1;
}

static int client_getname(lua_State *L) {
	int count = lua_gettop(L);
	for(int i = 1; i <= count; i++) {
		cs_str name = (cs_str)luaL_checkstring(L, i);
		lua_pushclient(L, Client_GetByName(name));
	}
	return count;
}

static int client_getcount(lua_State *L) {
	EPlayerState state = (EPlayerState)luaL_checkinteger(L, 1);
	lua_pushinteger(L, Clients_GetCount(state));
	return 1;
}

static int client_iterall(lua_State *L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);

	for(ClientID i = 0; i < MAX_CLIENTS; i++) {
		Client *client = Clients_List[i];
		if(client) {
			lua_pushvalue(L, 1);
			lua_pushclient(L, client);
			if(lua_pcall(L, 1, 0, 0) != 0) {
				lua_error(L);
				return 0;
			}
		}
	}

	return 0;
}

static const luaL_Reg clientlib[] ={
	{"getbyid", client_get},
	{"getbyname", client_getname},
	{"getbroadcast", client_getbcast},
	{"getcount", client_getcount},
	{"iterall", client_iterall},
	{NULL, NULL}
};

int luaopen_client(lua_State *L) {
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "__clients");

	luaL_newmetatable(L, "Client");
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

	luaL_register(L, luaL_checkstring(L, 1), clientlib);
	return 1;
}
