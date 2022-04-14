#include <core.h>
#include <str.h>
#include <client.h>
#include "luascript.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luaworld.h"
#include "luacolor.h"
#include "luacuboid.h"

Client *lua_checkclient(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, CSLUA_MCLIENT);
	luaL_argcheck(L, *ud != NULL, idx, "Invalid client");
	return (Client *)*ud;
}

Client *lua_optclient(lua_State *L, int idx) {
	void **ud = luaL_testudata(L, idx, CSLUA_MCLIENT);
	return ud ? *ud : NULL;
}

void lua_pushclient(lua_State *L, Client *client) {
	if(!client) {
		lua_pushnil(L);
		return;
	}

	ClientID id = Client_GetID(client);
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCLIENTS);
	lua_pushnumber(L, id);
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushnumber(L, id);
		lua_newuserdata(L, sizeof(Client *));
		luaL_setmetatable(L, CSLUA_MCLIENT);
		lua_settable(L, -3);
		lua_pushnumber(L, id);
		lua_gettable(L, -2);
	}

	lua_remove(L, -2);
	void **ud = lua_touserdata(L, -1);
	*ud = client;
}

void lua_clearclient(lua_State *L, Client *client) {
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RCLIENTS);
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
	lua_pushinteger(L, (lua_Integer)Client_GetID(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getstate(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetState(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getaddr(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_uint32 addr = Client_GetAddr(client);

	if(lua_gettop(L) > 1 && lua_toboolean(L, 2))
		lua_pushinteger(L, (lua_Integer)Client_GetAddr(client));
	else
		lua_pushfstring(L, "%d.%d.%d.%d",
			(addr & 0xFF),
			(addr >> 8) & 0xFF,
			(addr >> 16) & 0xFF,
			(addr >> 24) & 0xFF
		);
	return 1;
}

static int meta_getping(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	if(lua_gettop(L) > 1 && lua_toboolean(L, 2))
		lua_pushnumber(L, (lua_Number)Client_GetAvgPing(client));
	else
		lua_pushinteger(L, (lua_Integer)Client_GetPing(client));
	return 1;
}

static int meta_getname(lua_State *L) {
	lua_pushstring(L, Client_GetName(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getskin(lua_State *L) {
	lua_pushstring(L, Client_GetSkin(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getheldblock(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetHeldBlock(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getdispname(lua_State *L) {
	lua_pushstring(L, Client_GetDisplayName(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getappname(lua_State *L) {
	lua_pushstring(L, Client_GetAppName(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getposition(lua_State *L) {
	lua_pushboolean(L, Client_GetPosition(
		lua_checkclient(L, 1),
		lua_checkfloatvector(L, 2),
		NULL
	));
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
	lua_pushboolean(L, Client_GetPosition(
		lua_checkclient(L, 1),
		NULL, lua_checkangle(L, 2)
	));
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

static int meta_getfluidlvl(lua_State *L) {
	BlockID fluid = BLOCK_AIR;
	lua_pushinteger(L, (lua_Integer)Client_GetFluidLevel(
		lua_checkclient(L, 1),
		&fluid
	));
	lua_pushinteger(L, fluid);
	return 2;
}

static int meta_getstandblock(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetStandBlock(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getclickdist(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetClickDistance(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getmodel(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetModel(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getworld(lua_State *L) {
	lua_pushworld(L, Client_GetWorld(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_setop(lua_State *L) {
	lua_pushboolean(L, Client_SetOP(
		lua_checkclient(L, 1),
		(cs_bool)lua_toboolean(L, 2)
	));
	return 1;
}

static int meta_setspawn(lua_State *L) {
	lua_pushboolean(L, Client_SetSpawn(
		lua_checkclient(L, 1),
		lua_checkfloatvector(L, 2),
		lua_checkangle(L, 3)
	));
	return 1;
}

static int meta_setgroup(lua_State *L) {
	lua_pushboolean(L, Client_SetGroup(
		lua_checkclient(L, 1),
		(cs_uintptr)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setrotation(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushboolean(L,
		Client_SetProp(client, ENTITY_PROP_ROT_X, (cs_int32)luaL_checkinteger(L, 2)) &&
		Client_SetProp(client, ENTITY_PROP_ROT_Y, (cs_int32)luaL_checkinteger(L, 3)) &&
		Client_SetProp(client, ENTITY_PROP_ROT_Z, (cs_int32)luaL_checkinteger(L, 4))
	);
	return 1;
}

static int meta_setweather(lua_State *L) {
	lua_pushboolean(L, Client_SetWeather(
		lua_checkclient(L, 1),
		(EWeather)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setenvprop(lua_State *L) {
	lua_pushboolean(L, Client_SetEnvProperty(
		lua_checkclient(L, 1),
		(EProp)luaL_checkinteger(L, 2),
		(cs_int32)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setenvcolor(lua_State *L) {
	lua_pushboolean(L, Client_SetEnvColor(
		lua_checkclient(L, 1),
		(EColor)luaL_checkinteger(L, 2),
		lua_checkcolor3(L, 3)
	));
	return 1;
}

static int meta_setclickdist(lua_State *L) {
	lua_pushboolean(L, Client_SetClickDistance(
		lua_checkclient(L, 1),
		(cs_int16)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setblockperm(lua_State *L) {
	lua_pushboolean(L, Client_SetBlockPerm(
		lua_checkclient(L, 1),
		(BlockID)luaL_checkinteger(L, 2),
		(cs_bool)lua_toboolean(L, 3),
		(cs_bool)lua_toboolean(L, 4)
	));
	return 1;
}

static int meta_settextcolor(lua_State *L) {
	lua_pushboolean(L, Client_AddTextColor(
		lua_checkclient(L, 1),
		lua_checkcolor4(L, 2),
		*luaL_checkstring(L, 3)
	));
	return 1;
}

static int meta_setmotd(lua_State *L) {
	lua_pushboolean(L, Client_SetServerIdent(
		lua_checkclient(L, 1),
		luaL_checkstring(L, 2),
		luaL_checkstring(L, 3)
	));
	return 1;
}

static int meta_sethotkey(lua_State *L) {
	lua_pushboolean(L, Client_SetHotkey(
		lua_checkclient(L, 1),
		luaL_checkstring(L, 2),
		(cs_int32)luaL_checkinteger(L, 3),
		(cs_int8)luaL_checkinteger(L, 4)
	));
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

static int meta_setdispname(lua_State *L) {
	lua_pushboolean(L, Client_SetDisplayName(
		lua_checkclient(L, 1),
		luaL_checkstring(L, 2)
	));
	return 1;
}

static int meta_sethotbar(lua_State *L) {
	lua_pushboolean(L, Client_SetHotbar(
		lua_checkclient(L, 1),
		(cs_byte)luaL_checkinteger(L, 2),
		(BlockID)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_sethacks(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);
	CPEHacks hacks;

	lua_getfield(L, 2, "jumpheight");
	lua_getfield(L, 2, "thirdperson");
	lua_getfield(L, 2, "spawncontrol");
	lua_getfield(L, 2, "speeding");
	lua_getfield(L, 2, "noclip");
	lua_getfield(L, 2, "flying");

	hacks.flying = (cs_bool)lua_toboolean(L, -1);
	hacks.noclip = (cs_bool)lua_toboolean(L, -2);
	hacks.speeding = (cs_bool)lua_toboolean(L, -3);
	hacks.spawnControl = (cs_bool)lua_toboolean(L, -4);
	hacks.tpv = (cs_bool)lua_toboolean(L, -5);
	hacks.jumpHeight = (cs_int16)luaL_optinteger(L, -6, -1);
	lua_pop(L, 6);

	lua_pushboolean(L, Client_SendHacks(client, &hacks));
	return 1;
}

static int meta_setheldblock(lua_State *L) {
	lua_pushboolean(L, Client_SetHeldBlock(
		lua_checkclient(L, 1),
		(BlockID)luaL_checkinteger(L, 2),
		(cs_bool)lua_toboolean(L, 3)
	));
	return 1;
}

static int meta_setorderblock(lua_State *L) {
	lua_pushboolean(L, Client_SetInvOrder(
		lua_checkclient(L, 1),
		(cs_byte)luaL_checkinteger(L, 2),
		(BlockID)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setskin(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_str url = (cs_str)luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(url) < 65, 2, "URL is too long");
	lua_pushboolean(L, Client_SetSkin(client, url));
	return 1;
}

static int meta_settexpack(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	cs_str url = (cs_str)luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(url) < 65, 2, "URL is too long");
	lua_pushboolean(L, Client_SetTexturePack(client, url));
	return 1;
}

static int meta_setvelocity(lua_State *L) {
	lua_pushboolean(L, Client_SetVelocity(
		lua_checkclient(L, 1),
		lua_checkfloatvector(L, 2),
		true
	));
	return 1;
}

static int meta_gotoworld(lua_State *L) {
	lua_pushboolean(L, Client_ChangeWorld(
		lua_checkclient(L, 1),
		lua_checkworld(L, 2)
	));
	return 1;
}

static int meta_reload(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	World *world = Client_GetWorld(client);
	lua_pushboolean(L, Client_ChangeWorld(client, world));
	return 1;
}

static int meta_islocal(lua_State *L) {
	lua_pushboolean(L, Client_IsLocal(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isspawned(lua_State *L) {
	lua_pushboolean(L, Client_IsSpawned(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isinsameworld(lua_State *L) {
	lua_pushboolean(L, Client_IsInSameWorld(
		lua_checkclient(L, 1),
		lua_checkclient(L, 2)
	));
	return 1;
}

static int meta_isfirstspawn(lua_State *L) {
	lua_pushboolean(L, Client_IsFirstSpawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isinstate(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	int top = lua_gettop(L);
	for(int i = 2; i <= top; i++) {
		EClientState state = (EClientState)luaL_checkinteger(L, i);
		if(Client_CheckState(client, state)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	
	lua_pushboolean(L, 0);
	return 1;
}

static int meta_isbot(lua_State *L) {
	lua_pushboolean(L, Client_IsBot(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isop(lua_State *L) {
	lua_pushboolean(L, Client_IsOP(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_spawn(lua_State *L) {
	lua_pushboolean(L, Client_Spawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_despawn(lua_State *L) {
	lua_pushboolean(L, Client_Despawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_newcuboid(lua_State *L) {
	Client *client = lua_checkclient(L, 1);
	lua_newcubref(L, client, Client_NewSelection(client));
	return 1;
}

static int meta_update(lua_State *L) {
	lua_pushboolean(L, Client_Update(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_teleport(lua_State *L) {
	lua_pushboolean(L, Client_TeleportTo(
		lua_checkclient(L, 1),
		lua_checkfloatvector(L, 2),
		lua_checkangle(L, 3)
	));
	return 1;
}

static int meta_tospawn(lua_State *L) {
	lua_pushboolean(L, Client_TeleportToSpawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_kick(lua_State *L) {
	Client_Kick(
		lua_checkclient(L, 1),
		luaL_optstring(L, 2, NULL)
	);
	return 0;
}

static int meta_chat(lua_State *L) {
	Client *client = lua_optclient(L, 1);
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

static int meta_tostring(lua_State *L) {
	lua_pushfstring(L, "Client(%p)",
		lua_checkclient(L, 1)
	);
	return 1;
}

static const luaL_Reg clientmeta[] = {
	{"getid", meta_getid},
	{"getstate", meta_getstate},
	{"getaddr", meta_getaddr},
	{"getping", meta_getping},
	{"getname", meta_getname},
	{"getskin", meta_getskin},
	{"getheldblock", meta_getheldblock},
	{"getdispname", meta_getdispname},
	{"getappname", meta_getappname},
	{"getposition", meta_getposition},
	{"getpositiona", meta_getpositiona},
	{"getrotation", meta_getrotation},
	{"getrotationa", meta_getrotationa},
	{"getclickdist", meta_getclickdist},
	{"getfluidlvl", meta_getfluidlvl},
	{"getstandblock", meta_getstandblock},
	{"getmodel", meta_getmodel},
	{"getworld", meta_getworld},

	{"setop", meta_setop},
	{"setspawn", meta_setspawn},
	{"setgroup", meta_setgroup},
	{"setrotation", meta_setrotation},
	{"setweather", meta_setweather},
	{"setenvprop", meta_setenvprop},
	{"setenvcolor", meta_setenvcolor},
	{"setclickdist", meta_setclickdist},
	{"setblockperm", meta_setblockperm},
	{"settextcolor", meta_settextcolor},
	{"setmotd", meta_setmotd},
	{"sethotkey", meta_sethotkey},
	{"setmodel", meta_setmodel},
	{"setdispname", meta_setdispname},
	{"sethotbar", meta_sethotbar},
	{"sethacks", meta_sethacks},
	{"setheldblock", meta_setheldblock},
	{"setorderblock", meta_setorderblock},
	{"setskin", meta_setskin},
	{"settexpack", meta_settexpack},
	{"setvelocity", meta_setvelocity},
	{"gotoworld", meta_gotoworld},
	{"reload", meta_reload},

	{"islocal", meta_islocal},
	{"isspawned", meta_isspawned},
	{"isinsameworld", meta_isinsameworld},
	{"isfirstspawn", meta_isfirstspawn},
	{"isinstate", meta_isinstate},
	{"isbot", meta_isbot},
	{"isop", meta_isop},

	{"spawn", meta_spawn},
	{"despawn", meta_despawn},
	{"newcuboid", meta_newcuboid},
	{"update", meta_update},
	{"teleport", meta_teleport},
	{"tospawn", meta_tospawn},
	{"kick", meta_kick},
	{"chat", meta_chat},

	{"__tostring", meta_tostring},

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

static int client_getname(lua_State *L) {
	int count = lua_gettop(L);
	for(int i = 1; i <= count; i++) {
		cs_str name = (cs_str)luaL_checkstring(L, i);
		lua_pushclient(L, Client_GetByName(name));
	}
	return count;
}

static int client_getcount(lua_State *L) {
	EClientState state = (EClientState)luaL_checkinteger(L, 1);
	lua_pushinteger(L, Clients_GetCount(state));
	return 1;
}

static int client_iterall(lua_State *L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);

	for(ClientID i = 0; i < MAX_CLIENTS; i++) {
		Client *client = Clients_List[i];
		if(client && !Client_IsBot(client)) {
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

static int client_newbot(lua_State *L) {
	lua_pushclient(L, Client_NewBot());
	return 1;
}

static const luaL_Reg clientlib[] = {
	{"getbyid", client_get},
	{"getbyname", client_getname},
	{"getcount", client_getcount},
	{"iterall", client_iterall},
	{"newbot", client_newbot},
	{NULL, NULL}
};

int luaopen_client(lua_State *L) {
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, CSLUA_RCLIENTS);

	luaL_newmetatable(L, CSLUA_MCLIENT);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, clientmeta, 0);
	lua_pop(L, 1);

	lua_addintconst(L, MESSAGE_TYPE_CHAT);
	lua_addintconst(L, MESSAGE_TYPE_STATUS1);
	lua_addintconst(L, MESSAGE_TYPE_STATUS2);
	lua_addintconst(L, MESSAGE_TYPE_STATUS3);
	lua_addintconst(L, MESSAGE_TYPE_BRIGHT1);
	lua_addintconst(L, MESSAGE_TYPE_BRIGHT2);
	lua_addintconst(L, MESSAGE_TYPE_BRIGHT3);
	lua_addintconst(L, MESSAGE_TYPE_ANNOUNCE);

	lua_addintconst(L, CLIENT_STATE_INITIAL);
	lua_addintconst(L, CLIENT_STATE_MOTD);
	lua_addintconst(L, CLIENT_STATE_INGAME);

	lua_addintconst(L, ENTITY_PROP_ROT_X);
	lua_addintconst(L, ENTITY_PROP_ROT_Y);
	lua_addintconst(L, ENTITY_PROP_ROT_Z);

	luaL_register(L, luaL_checkstring(L, 1), clientlib);
	*(void **)lua_newuserdata(L, sizeof(Client *)) = CLIENT_BROADCAST;
	luaL_setmetatable(L, CSLUA_MCLIENT);
	lua_setfield(L, -2, "broadcast");
	return 1;
}
