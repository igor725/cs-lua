#include <core.h>
#include <str.h>
#include <client.h>
#include <types/keys.h>
#include "luascript.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luaworld.h"
#include "luacolor.h"
#include "luacuboid.h"
#include "luablock.h"

Client *lua_checkclient(scr_Context *L, int idx) {
	void **ud = luaL_checkudata(L, idx, CSLUA_MCLIENT);
	luaL_argcheck(L, *ud != NULL, idx, "Invalid client");
	return (Client *)*ud;
}

Client *lua_toclient(scr_Context *L, int idx) {
	void **ud = luaL_testudata(L, idx, CSLUA_MCLIENT);
	return ud ? *ud : NULL;
}

void lua_pushclient(scr_Context *L, Client *client) {
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

void lua_clearclient(scr_Context *L, Client *client) {
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

static int meta_getid(scr_Context *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetID(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getstate(scr_Context *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetState(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getaddr(scr_Context *L) {
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

static int meta_getping(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	if(lua_gettop(L) > 1 && lua_toboolean(L, 2))
		lua_pushnumber(L, (lua_Number)Client_GetAvgPing(client));
	else
		lua_pushinteger(L, (lua_Integer)Client_GetPing(client));
	return 1;
}

static int meta_getname(scr_Context *L) {
	lua_pushstring(L, Client_GetName(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getskin(scr_Context *L) {
	lua_pushstring(L, Client_GetSkin(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getgroup(scr_Context *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetGroupID(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getheldblock(scr_Context *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetHeldBlock(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getdispname(scr_Context *L) {
	lua_pushstring(L, Client_GetDisplayName(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getappname(scr_Context *L) {
	lua_pushstring(L, Client_GetAppName(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getposition(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	Vec *vector = lua_tofloatvector(L, 2);

	if(!vector) {
		LuaVector *lvec = lua_newvector(L);
		lvec->type = LUAVECTOR_TFLOAT;
		vector = &lvec->value.f;
	}

	Client_GetPosition(client, vector, NULL);
	return 1;
}

static int meta_getrotation(scr_Context *L) {
	Ang *angle = lua_toangle(L, 2);
	if(!angle) angle = lua_newangle(L);

	Client_GetPosition(
		lua_checkclient(L, 1),
		NULL, angle
	);

	return 1;
}

static int meta_getfluidlvl(scr_Context *L) {
	BlockID fluid = BLOCK_AIR;
	lua_pushinteger(L, (lua_Integer)Client_GetFluidLevel(
		lua_checkclient(L, 1),
		&fluid
	));
	lua_pushinteger(L, fluid);
	return 2;
}

static int meta_getstandblock(scr_Context *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetStandBlock(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getclickdist(scr_Context *L) {
	if(!lua_toboolean(L, 2))
		lua_pushinteger(L, (lua_Integer)Client_GetClickDistance(
			lua_checkclient(L, 1)
		));
	else
		lua_pushnumber(L, (lua_Number)Client_GetClickDistanceInBlocks(
			lua_checkclient(L, 1)
		));
	return 1;
}

static int meta_getmodel(scr_Context *L) {
	lua_pushinteger(L, (lua_Integer)Client_GetModel(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_getworld(scr_Context *L) {
	lua_pushworld(L, Client_GetWorld(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_setop(scr_Context *L) {
	lua_pushboolean(L, Client_SetOP(
		lua_checkclient(L, 1),
		(cs_bool)lua_toboolean(L, 2)
	));
	return 1;
}

static int meta_setspawn(scr_Context *L) {
	lua_pushboolean(L, Client_SetSpawn(
		lua_checkclient(L, 1),
		lua_checkfloatvector(L, 2),
		lua_checkangle(L, 3)
	));
	return 1;
}

static int meta_setgroup(scr_Context *L) {
	lua_pushboolean(L, Client_SetGroup(
		lua_checkclient(L, 1),
		(cs_uintptr)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setmdlrotation(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	lua_pushboolean(L,
		Client_SetProp(client, ENTITY_PROP_ROT_X, (cs_int32)luaL_checkinteger(L, 2)) &&
		Client_SetProp(client, ENTITY_PROP_ROT_Y, (cs_int32)luaL_checkinteger(L, 3)) &&
		Client_SetProp(client, ENTITY_PROP_ROT_Z, (cs_int32)luaL_checkinteger(L, 4))
	);
	return 1;
}

static int meta_setweather(scr_Context *L) {
	lua_pushboolean(L, Client_SetWeather(
		lua_checkclient(L, 1),
		(EWeather)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setenvprop(scr_Context *L) {
	lua_pushboolean(L, Client_SetEnvProperty(
		lua_checkclient(L, 1),
		(EProp)luaL_checkinteger(L, 2),
		(cs_int32)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setenvcolor(scr_Context *L) {
	lua_pushboolean(L, Client_SetEnvColor(
		lua_checkclient(L, 1),
		(EColor)luaL_checkinteger(L, 2),
		lua_checkcolor3(L, 3)
	));
	return 1;
}

static int meta_setclickdist(scr_Context *L) {
	lua_pushboolean(L, Client_SetClickDistance(
		lua_checkclient(L, 1),
		(cs_int16)luaL_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setblockperm(scr_Context *L) {
	lua_pushboolean(L, Client_SetBlockPerm(
		lua_checkclient(L, 1),
		(BlockID)luaL_checkinteger(L, 2),
		(cs_bool)lua_toboolean(L, 3),
		(cs_bool)lua_toboolean(L, 4)
	));
	return 1;
}

static int meta_settextcolor(scr_Context *L) {
	lua_pushboolean(L, Client_AddTextColor(
		lua_checkclient(L, 1),
		lua_checkcolor4(L, 2),
		*luaL_checkstring(L, 3)
	));
	return 1;
}

static int meta_setmotd(scr_Context *L) {
	lua_pushboolean(L, Client_SetServerIdent(
		lua_checkclient(L, 1),
		luaL_checkstring(L, 2),
		luaL_checkstring(L, 3)
	));
	return 1;
}

static int meta_sethotkey(scr_Context *L) {
	lua_pushboolean(L, Client_SetHotkey(
		lua_checkclient(L, 1),
		luaL_checkstring(L, 2),
		(ELWJGLKey)luaL_checkinteger(L, 3),
		(ELWJGLMod)luaL_checkinteger(L, 4)
	));
	return 1;
}

static int meta_setmodel(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	if(lua_isnumber(L, 2))
		lua_pushboolean(L, Client_SetModel(client, (cs_int16)luaL_checkinteger(L, 2)));
	else
		lua_pushboolean(L, Client_SetModelStr(client, luaL_checkstring(L, 2)));
	return 1;
}

static int meta_setdispname(scr_Context *L) {
	lua_pushboolean(L, Client_SetDisplayName(
		lua_checkclient(L, 1),
		luaL_checkstring(L, 2)
	));
	return 1;
}

static int meta_sethotbar(scr_Context *L) {
	lua_pushboolean(L, Client_SetHotbar(
		lua_checkclient(L, 1),
		(cs_byte)luaL_checkinteger(L, 2),
		(BlockID)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_sethacks(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	lua_getfield(L, 2, "jumpheight");
	lua_getfield(L, 2, "thirdperson");
	lua_getfield(L, 2, "spawncontrol");
	lua_getfield(L, 2, "speeding");
	lua_getfield(L, 2, "noclip");
	lua_getfield(L, 2, "flying");

	CPEHacks hacks = {
		.flying = (cs_bool)lua_toboolean(L, -1),
		.noclip = (cs_bool)lua_toboolean(L, -2),
		.speeding = (cs_bool)lua_toboolean(L, -3),
		.spawnControl = (cs_bool)lua_toboolean(L, -4),
		.tpv = (cs_bool)lua_toboolean(L, -5),
		.jumpHeight = (cs_int16)luaL_optinteger(L, -6, -1)
	};
	lua_pop(L, 6);

	lua_pushboolean(L, Client_SendHacks(client, &hacks));
	return 1;
}

static int meta_setheldblock(scr_Context *L) {
	lua_pushboolean(L, Client_SetHeldBlock(
		lua_checkclient(L, 1),
		(BlockID)luaL_checkinteger(L, 2),
		(cs_bool)lua_toboolean(L, 3)
	));
	return 1;
}

static int meta_setorderblock(scr_Context *L) {
	lua_pushboolean(L, Client_SetInvOrder(
		lua_checkclient(L, 1),
		(cs_byte)luaL_checkinteger(L, 2),
		(BlockID)luaL_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setskin(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	cs_str url = (cs_str)luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(url) < MAX_STR_LEN, 2, "URL is too long");
	lua_pushboolean(L, Client_SetSkin(client, url));
	return 1;
}

static int meta_settexpack(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	cs_str url = (cs_str)luaL_checkstring(L, 2);
	luaL_argcheck(L, String_Length(url) < MAX_STR_LEN, 2, "URL is too long");
	lua_pushboolean(L, Client_SetTexturePack(client, url));
	return 1;
}

static int meta_setvelocity(scr_Context *L) {
	lua_pushboolean(L, Client_SetVelocity(
		lua_checkclient(L, 1),
		lua_checkfloatvector(L, 2),
		(cs_byte)luaL_optinteger(L, 3, 0)
	));
	return 1;
}

static int meta_gotoworld(scr_Context *L) {
	lua_pushboolean(L, Client_ChangeWorld(
		lua_checkclient(L, 1),
		lua_checkworld(L, 2)
	));
	return 1;
}

static int meta_reload(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	World *world = Client_GetWorld(client);
	lua_pushboolean(L, Client_ChangeWorld(client, world));
	return 1;
}

static int meta_islocal(scr_Context *L) {
	lua_pushboolean(L, Client_IsLocal(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isspawned(scr_Context *L) {
	lua_pushboolean(L, Client_IsSpawned(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isinsameworld(scr_Context *L) {
	lua_pushboolean(L, Client_IsInSameWorld(
		lua_checkclient(L, 1),
		lua_checkclient(L, 2)
	));
	return 1;
}

static int meta_isfirstspawn(scr_Context *L) {
	lua_pushboolean(L, Client_IsFirstSpawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isinstate(scr_Context *L) {
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

static int meta_isbot(scr_Context *L) {
	lua_pushboolean(L, Client_IsBot(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_isop(scr_Context *L) {
	lua_pushboolean(L, Client_IsOP(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_spawn(scr_Context *L) {
	lua_pushboolean(L, Client_Spawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_despawn(scr_Context *L) {
	lua_pushboolean(L, Client_Despawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_sendbulk(scr_Context *L) {
	Client_BulkBlockUpdate(
		lua_checkclient(L, 1),
		lua_checkbulk(L, 2)
	);
	return 0;
}

static int meta_particle(scr_Context *L) {
	Vec *position = lua_checkfloatvector(L, 3);
	Vec *origin = lua_tofloatvector(L, 4);
	if(!origin) origin = position;
	lua_pushboolean(L, Client_SpawnParticle(
		lua_checkclient(L, 1),
		(cs_byte)luaL_checkinteger(L, 2),
		position, origin
	));
	return 1;
}

static int meta_newcuboid(scr_Context *L) {
	Client *client = lua_checkclient(L, 1);
	lua_newcubref(L, client, Client_NewSelection(client));
	return 1;
}

static int meta_plmesg(scr_Context *L) {
	lua_pushboolean(L, Client_SendPluginMessage(
		lua_checkclient(L, 1),
		(cs_byte)luaL_checkinteger(L, 2),
		luaL_checkstring(L, 3)
	));
	return 1;
}

static int meta_update(scr_Context *L) {
	lua_pushboolean(L, Client_Update(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_teleport(scr_Context *L) {
	if (lua_isnumber(L, 2)) {
		lua_pushboolean(L, Client_ExtTeleportTo(
			lua_checkclient(L, 1),
			(cs_byte)luaL_checkinteger(L, 2),
			lua_checkfloatvector(L, 3),
			lua_checkangle(L, 4)
		));
	} else
		lua_pushboolean(L, Client_TeleportTo(
			lua_checkclient(L, 1),
			lua_checkfloatvector(L, 2),
			lua_checkangle(L, 3)
		));
	return 1;
}

static int meta_tospawn(scr_Context *L) {
	lua_pushboolean(L, Client_TeleportToSpawn(
		lua_checkclient(L, 1)
	));
	return 1;
}

static int meta_kick(scr_Context *L) {
	Client_Kick(
		lua_checkclient(L, 1),
		luaL_optstring(L, 2, NULL)
	);
	return 0;
}

static int meta_chat(scr_Context *L) {
	Client *client = lua_toclient(L, 1);
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
	{"getstate", meta_getstate},
	{"getaddr", meta_getaddr},
	{"getping", meta_getping},
	{"getname", meta_getname},
	{"getskin", meta_getskin},
	{"getgroup", meta_getgroup},
	{"getheldblock", meta_getheldblock},
	{"getdispname", meta_getdispname},
	{"getappname", meta_getappname},
	{"getposition", meta_getposition},
	{"getrotation", meta_getrotation},
	{"getclickdist", meta_getclickdist},
	{"getfluidlvl", meta_getfluidlvl},
	{"getstandblock", meta_getstandblock},
	{"getmodel", meta_getmodel},
	{"getworld", meta_getworld},

	{"setop", meta_setop},
	{"setspawn", meta_setspawn},
	{"setgroup", meta_setgroup},
	{"setmdlrotation", meta_setmdlrotation},
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

	{"islocal", meta_islocal},
	{"isspawned", meta_isspawned},
	{"isinsameworld", meta_isinsameworld},
	{"isfirstspawn", meta_isfirstspawn},
	{"isinstate", meta_isinstate},
	{"isbot", meta_isbot},
	{"isop", meta_isop},

	{"spawn", meta_spawn},
	{"despawn", meta_despawn},
	{"sendbulk", meta_sendbulk},
	{"particle", meta_particle},
	{"newcuboid", meta_newcuboid},
	{"plmesg", meta_plmesg},
	{"update", meta_update},
	{"teleport", meta_teleport},
	{"tospawn", meta_tospawn},
	{"gotoworld", meta_gotoworld},
	{"reload", meta_reload},
	{"kick", meta_kick},
	{"chat", meta_chat},

	{NULL, NULL}
};

static int client_get(scr_Context *L) {
	int count = lua_gettop(L);
	for(int i = 1; i <= count; i++)
		lua_pushclient(L, Client_GetByID(
			(ClientID)luaL_checknumber(L, i)
		));
	return count;
}

static int client_getname(scr_Context *L) {
	int count = lua_gettop(L);
	for(int i = 1; i <= count; i++)
		lua_pushclient(L, Client_GetByName(
			(cs_str)luaL_checkstring(L, i)
		));
	return count;
}

static int client_getcount(scr_Context *L) {
	lua_pushinteger(L, Clients_GetCount(
		(EClientState)luaL_checkinteger(L, 1)
	));
	return 1;
}

static int client_iterall(scr_Context *L) {
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

static int client_newbot(scr_Context *L) {
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

int luaopen_client(scr_Context *L) {
	lua_createtable(L, MAX_CLIENTS, 0);
	lua_setfield(L, LUA_REGISTRYINDEX, CSLUA_RCLIENTS);
	lua_indexedmeta(L, CSLUA_MCLIENT, clientmeta);

	lua_addintconst(L, MESSAGE_TYPE_CHAT);
	lua_addintconst(L, MESSAGE_TYPE_STATUS1);
	lua_addintconst(L, MESSAGE_TYPE_STATUS2);
	lua_addintconst(L, MESSAGE_TYPE_STATUS3);
	lua_addintconst(L, MESSAGE_TYPE_BRIGHT1);
	lua_addintconst(L, MESSAGE_TYPE_BRIGHT2);
	lua_addintconst(L, MESSAGE_TYPE_BRIGHT3);
	lua_addintconst(L, MESSAGE_TYPE_ANNOUNCE);
	lua_addintconst(L, MESSAGE_TYPE_BIGANNOUNCE);
	lua_addintconst(L, MESSAGE_TYPE_SMALLANNOUNCE);

	lua_addintconst(L, CLIENT_STATE_INITIAL);
	lua_addintconst(L, CLIENT_STATE_MOTD);
	lua_addintconst(L, CLIENT_STATE_INGAME);

	lua_addintconst(L, ENTITY_PROP_ROT_X);
	lua_addintconst(L, ENTITY_PROP_ROT_Y);
	lua_addintconst(L, ENTITY_PROP_ROT_Z);

	lua_addintconst(L, CPE_VELCTL_ADDALL);
	lua_addintconst(L, CPE_VELCTL_SETX);
	lua_addintconst(L, CPE_VELCTL_SETY);
	lua_addintconst(L, CPE_VELCTL_SETZ);
	lua_addintconst(L, CPE_VELCTL_SETALL);

	luaL_newlib(L, clientlib);
	*(void **)lua_newuserdata(L, sizeof(Client *)) = CLIENT_BROADCAST;
	luaL_setmetatable(L, CSLUA_MCLIENT);
	lua_setfield(L, -2, "broadcast");

	return 1;
}
