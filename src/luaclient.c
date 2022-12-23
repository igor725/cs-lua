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

Client *scr_checkclient(scr_Context *L, int idx) {
	void **ud = scr_checkmemtype(L, idx, CSSCRIPTS_MCLIENT);
	scr_argassert(L, *ud != NULL, idx, "Invalid client");
	return (Client *)*ud;
}

Client *scr_toclient(scr_Context *L, int idx) {
	void **ud = scr_testmemtype(L, idx, CSSCRIPTS_MCLIENT);
	return ud ? *ud : NULL;
}

void scr_pushclient(scr_Context *L, Client *client) {
	if(!client) {
		scr_pushnull(L);
		return;
	}

	ClientID id = Client_GetID(client);
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCLIENTS);
	scr_pushnumber(L, id);
	scr_getfromtable(L, -2);
	if(scr_isnull(L, -1)) {
		scr_stackpop(L, 1);
		scr_pushnumber(L, id);
		scr_allocmem(L, sizeof(Client *));
		scr_setmemtype(L, CSSCRIPTS_MCLIENT);
		scr_settotable(L, -3);
		scr_pushnumber(L, id);
		scr_getfromtable(L, -2);
	}

	scr_stackrem(L, -2);
	void **ud = scr_getmem(L, -1);
	*ud = client;
}

void scr_clearclient(scr_Context *L, Client *client) {
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCLIENTS);
	scr_pushinteger(L, Client_GetID(client));
	scr_getfromtable(L, -2);
	if(!scr_ismem(L, -1)) {
		scr_stackpop(L, 1);
		return;
	}
	void **ud = scr_getmem(L, -1);
	*ud = NULL;
}

static int meta_getid(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Client_GetID(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getstate(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Client_GetState(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getaddr(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	cs_uint32 addr = Client_GetAddr(client);

	if(scr_stacktop(L) > 1 && scr_toboolean(L, 2))
		scr_pushinteger(L, (scr_Integer)Client_GetAddr(client));
	else
		scr_pushformatstring(L, "%d.%d.%d.%d",
			(addr & 0xFF),
			(addr >> 8) & 0xFF,
			(addr >> 16) & 0xFF,
			(addr >> 24) & 0xFF
		);
	return 1;
}

static int meta_getping(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	if(scr_stacktop(L) > 1 && scr_toboolean(L, 2))
		scr_pushnumber(L, (scr_Number)Client_GetAvgPing(client));
	else
		scr_pushinteger(L, (scr_Integer)Client_GetPing(client));
	return 1;
}

static int meta_getname(scr_Context *L) {
	scr_pushstring(L, Client_GetName(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getskin(scr_Context *L) {
	scr_pushstring(L, Client_GetSkin(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getgroup(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Client_GetGroupID(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getheldblock(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Client_GetHeldBlock(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getdispname(scr_Context *L) {
	scr_pushstring(L, Client_GetDisplayName(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getappname(scr_Context *L) {
	scr_pushstring(L, Client_GetAppName(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getposition(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	Vec *vector = scr_tofloatvector(L, 2);

	if(!vector) {
		LuaVector *lvec = scr_newvector(L);
		lvec->type = LUAVECTOR_TFLOAT;
		vector = &lvec->value.f;
	}

	Client_GetPosition(client, vector, NULL);
	return 1;
}

static int meta_getrotation(scr_Context *L) {
	Ang *angle = scr_toangle(L, 2);
	if(!angle) angle = scr_newangle(L);

	Client_GetPosition(
		scr_checkclient(L, 1),
		NULL, angle
	);

	return 1;
}

static int meta_getfluidlvl(scr_Context *L) {
	BlockID fluid = BLOCK_AIR;
	scr_pushinteger(L, (scr_Integer)Client_GetFluidLevel(
		scr_checkclient(L, 1),
		&fluid
	));
	scr_pushinteger(L, fluid);
	return 2;
}

static int meta_getstandblock(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Client_GetStandBlock(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getclickdist(scr_Context *L) {
	if(!scr_toboolean(L, 2))
		scr_pushinteger(L, (scr_Integer)Client_GetClickDistance(
			scr_checkclient(L, 1)
		));
	else
		scr_pushnumber(L, (scr_Number)Client_GetClickDistanceInBlocks(
			scr_checkclient(L, 1)
		));
	return 1;
}

static int meta_getmodel(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Client_GetModel(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_getworld(scr_Context *L) {
	scr_pushworld(L, Client_GetWorld(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_setop(scr_Context *L) {
	scr_pushboolean(L, Client_SetOP(
		scr_checkclient(L, 1),
		scr_toboolean(L, 2)
	));
	return 1;
}

static int meta_setspawn(scr_Context *L) {
	scr_pushboolean(L, Client_SetSpawn(
		scr_checkclient(L, 1),
		scr_checkfloatvector(L, 2),
		scr_checkangle(L, 3)
	));
	return 1;
}

static int meta_setgroup(scr_Context *L) {
	scr_pushboolean(L, Client_SetGroup(
		scr_checkclient(L, 1),
		(cs_uintptr)scr_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setmdlrotation(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	scr_pushboolean(L,
		Client_SetProp(client, ENTITY_PROP_ROT_X, (cs_int32)scr_checkinteger(L, 2)) &&
		Client_SetProp(client, ENTITY_PROP_ROT_Y, (cs_int32)scr_checkinteger(L, 3)) &&
		Client_SetProp(client, ENTITY_PROP_ROT_Z, (cs_int32)scr_checkinteger(L, 4))
	);
	return 1;
}

static int meta_setweather(scr_Context *L) {
	scr_pushboolean(L, Client_SetWeather(
		scr_checkclient(L, 1),
		(EWeather)scr_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setenvprop(scr_Context *L) {
	scr_pushboolean(L, Client_SetEnvProperty(
		scr_checkclient(L, 1),
		(EProp)scr_checkinteger(L, 2),
		(cs_int32)scr_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setenvcolor(scr_Context *L) {
	scr_pushboolean(L, Client_SetEnvColor(
		scr_checkclient(L, 1),
		(EColor)scr_checkinteger(L, 2),
		scr_checkcolor3(L, 3)
	));
	return 1;
}

static int meta_setclickdist(scr_Context *L) {
	scr_pushboolean(L, Client_SetClickDistance(
		scr_checkclient(L, 1),
		(cs_int16)scr_checkinteger(L, 2)
	));
	return 1;
}

static int meta_setblockperm(scr_Context *L) {
	scr_pushboolean(L, Client_SetBlockPerm(
		scr_checkclient(L, 1),
		(BlockID)scr_checkinteger(L, 2),
		scr_toboolean(L, 3),
		scr_toboolean(L, 4)
	));
	return 1;
}

static int meta_settextcolor(scr_Context *L) {
	scr_pushboolean(L, Client_AddTextColor(
		scr_checkclient(L, 1),
		scr_checkcolor4(L, 2),
		*scr_checkstring(L, 3)
	));
	return 1;
}

static int meta_setmotd(scr_Context *L) {
	scr_pushboolean(L, Client_SetServerIdent(
		scr_checkclient(L, 1),
		scr_checkstring(L, 2),
		scr_checkstring(L, 3)
	));
	return 1;
}

static int meta_sethotkey(scr_Context *L) {
	scr_pushboolean(L, Client_SetHotkey(
		scr_checkclient(L, 1),
		scr_checkstring(L, 2),
		(ELWJGLKey)scr_checkinteger(L, 3),
		(ELWJGLMod)scr_checkinteger(L, 4)
	));
	return 1;
}

static int meta_setmodel(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	if(scr_isnumber(L, 2))
		scr_pushboolean(L, Client_SetModel(client, (cs_int16)scr_checkinteger(L, 2)));
	else
		scr_pushboolean(L, Client_SetModelStr(client, scr_checkstring(L, 2)));
	return 1;
}

static int meta_setdispname(scr_Context *L) {
	scr_pushboolean(L, Client_SetDisplayName(
		scr_checkclient(L, 1),
		scr_checkstring(L, 2)
	));
	return 1;
}

static int meta_sethotbar(scr_Context *L) {
	scr_pushboolean(L, Client_SetHotbar(
		scr_checkclient(L, 1),
		(cs_byte)scr_checkinteger(L, 2),
		(BlockID)scr_checkinteger(L, 3)
	));
	return 1;
}

static int meta_sethacks(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	scr_gettabfield(L, 2, "jumpheight");
	scr_gettabfield(L, 2, "thirdperson");
	scr_gettabfield(L, 2, "spawncontrol");
	scr_gettabfield(L, 2, "speeding");
	scr_gettabfield(L, 2, "noclip");
	scr_gettabfield(L, 2, "flying");

	CPEHacks hacks = {
		.flying = scr_toboolean(L, -1),
		.noclip = scr_toboolean(L, -2),
		.speeding = scr_toboolean(L, -3),
		.spawnControl = scr_toboolean(L, -4),
		.tpv = scr_toboolean(L, -5),
		.jumpHeight = (cs_int16)scr_optinteger(L, -6, -1)
	};
	scr_stackpop(L, 6);

	scr_pushboolean(L, Client_SendHacks(client, &hacks));
	return 1;
}

static int meta_setheldblock(scr_Context *L) {
	scr_pushboolean(L, Client_SetHeldBlock(
		scr_checkclient(L, 1),
		(BlockID)scr_checkinteger(L, 2),
		scr_toboolean(L, 3)
	));
	return 1;
}

static int meta_setorderblock(scr_Context *L) {
	scr_pushboolean(L, Client_SetInvOrder(
		scr_checkclient(L, 1),
		(cs_byte)scr_checkinteger(L, 2),
		(BlockID)scr_checkinteger(L, 3)
	));
	return 1;
}

static int meta_setskin(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	cs_str url = (cs_str)scr_checkstring(L, 2);
	scr_argassert(L, String_Length(url) < MAX_STR_LEN, 2, "URL is too long");
	scr_pushboolean(L, Client_SetSkin(client, url));
	return 1;
}

static int meta_settexpack(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	cs_str url = (cs_str)scr_checkstring(L, 2);
	scr_argassert(L, String_Length(url) < MAX_STR_LEN, 2, "URL is too long");
	scr_pushboolean(L, Client_SetTexturePack(client, url));
	return 1;
}

static int meta_setvelocity(scr_Context *L) {
	scr_pushboolean(L, Client_SetVelocity(
		scr_checkclient(L, 1),
		scr_checkfloatvector(L, 2),
		(cs_byte)scr_optinteger(L, 3, 0)
	));
	return 1;
}

static int meta_gotoworld(scr_Context *L) {
	scr_pushboolean(L, Client_ChangeWorld(
		scr_checkclient(L, 1),
		scr_checkworld(L, 2)
	));
	return 1;
}

static int meta_reload(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	World *world = Client_GetWorld(client);
	scr_pushboolean(L, Client_ChangeWorld(client, world));
	return 1;
}

static int meta_islocal(scr_Context *L) {
	scr_pushboolean(L, Client_IsLocal(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_isspawned(scr_Context *L) {
	scr_pushboolean(L, Client_IsSpawned(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_isinsameworld(scr_Context *L) {
	scr_pushboolean(L, Client_IsInSameWorld(
		scr_checkclient(L, 1),
		scr_checkclient(L, 2)
	));
	return 1;
}

static int meta_isfirstspawn(scr_Context *L) {
	scr_pushboolean(L, Client_IsFirstSpawn(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_isinstate(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	int top = scr_stacktop(L);
	for(int i = 2; i <= top; i++) {
		EClientState state = (EClientState)scr_checkinteger(L, i);
		if(Client_CheckState(client, state)) {
			scr_pushboolean(L, 1);
			return 1;
		}
	}

	scr_pushboolean(L, 0);
	return 1;
}

static int meta_isbot(scr_Context *L) {
	scr_pushboolean(L, Client_IsBot(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_isop(scr_Context *L) {
	scr_pushboolean(L, Client_IsOP(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_spawn(scr_Context *L) {
	scr_pushboolean(L, Client_Spawn(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_despawn(scr_Context *L) {
	scr_pushboolean(L, Client_Despawn(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_sendbulk(scr_Context *L) {
	Client_BulkBlockUpdate(
		scr_checkclient(L, 1),
		scr_checkbulk(L, 2)
	);
	return 0;
}

static int meta_particle(scr_Context *L) {
	Vec *position = scr_checkfloatvector(L, 3);
	Vec *origin = scr_tofloatvector(L, 4);
	if(!origin) origin = position;
	scr_pushboolean(L, Client_SpawnParticle(
		scr_checkclient(L, 1),
		(cs_byte)scr_checkinteger(L, 2),
		position, origin
	));
	return 1;
}

static int meta_newcuboid(scr_Context *L) {
	Client *client = scr_checkclient(L, 1);
	scr_newcubref(L, client, Client_NewSelection(client));
	return 1;
}

static int meta_plmesg(scr_Context *L) {
	scr_pushboolean(L, Client_SendPluginMessage(
		scr_checkclient(L, 1),
		(cs_byte)scr_checkinteger(L, 2),
		scr_checkstring(L, 3)
	));
	return 1;
}

static int meta_update(scr_Context *L) {
	scr_pushboolean(L, Client_Update(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_teleport(scr_Context *L) {
	if (scr_isnumber(L, 2)) {
		scr_pushboolean(L, Client_ExtTeleportTo(
			scr_checkclient(L, 1),
			(cs_byte)scr_checkinteger(L, 2),
			scr_checkfloatvector(L, 3),
			scr_checkangle(L, 4)
		));
	} else
		scr_pushboolean(L, Client_TeleportTo(
			scr_checkclient(L, 1),
			scr_checkfloatvector(L, 2),
			scr_checkangle(L, 3)
		));
	return 1;
}

static int meta_tospawn(scr_Context *L) {
	scr_pushboolean(L, Client_TeleportToSpawn(
		scr_checkclient(L, 1)
	));
	return 1;
}

static int meta_kick(scr_Context *L) {
	Client_Kick(
		scr_checkclient(L, 1),
		luaL_optstring(L, 2, NULL)
	);
	return 0;
}

static int meta_chat(scr_Context *L) {
	Client *client = scr_toclient(L, 1);
	EMesgType type = MESSAGE_TYPE_CHAT;
	cs_str mesg = NULL;

	if(scr_stacktop(L) == 2)
		mesg = scr_checkstring(L, 2);
	else {
		type = (EMesgType)scr_checkinteger(L, 2);
		mesg = scr_checkstring(L, 3);
	}

	Client_Chat(client, type, mesg);
	return 0;
}

static const scr_RegFuncs clientmeta[] = {
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
	int count = scr_stacktop(L);
	for(int i = 1; i <= count; i++)
		scr_pushclient(L, Client_GetByID(
			(ClientID)scr_checknumber(L, i)
		));
	return count;
}

static int client_getname(scr_Context *L) {
	int count = scr_stacktop(L);
	for(int i = 1; i <= count; i++)
		scr_pushclient(L, Client_GetByName(
			(cs_str)scr_checkstring(L, i)
		));
	return count;
}

static int client_getcount(scr_Context *L) {
	scr_pushinteger(L, Clients_GetCount(
		(EClientState)scr_checkinteger(L, 1)
	));
	return 1;
}

static int client_iterall(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);

	for(ClientID i = 0; i < MAX_CLIENTS; i++) {
		Client *client = Clients_List[i];
		if(client && !Client_IsBot(client)) {
			scr_stackpush(L, 1);
			scr_pushclient(L, client);
			if(scr_protectedcall(L, 1, 0, 0) != 0) {
				scr_error(L);
				return 0;
			}
		}
	}

	return 0;
}

static int client_newbot(scr_Context *L) {
	scr_pushclient(L, Client_NewBot());
	return 1;
}

static const scr_RegFuncs clientlib[] = {
	{"getbyid", client_get},
	{"getbyname", client_getname},
	{"getcount", client_getcount},

	{"iterall", client_iterall},
	{"newbot", client_newbot},

	{NULL, NULL}
};

int scr_libfunc(client)(scr_Context *L) {
	scr_newntable(L, MAX_CLIENTS, 0);
	scr_settabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCLIENTS);
	scr_createtype(L, CSSCRIPTS_MCLIENT, clientmeta);

	scr_addintconst(L, MESSAGE_TYPE_CHAT);
	scr_addintconst(L, MESSAGE_TYPE_STATUS1);
	scr_addintconst(L, MESSAGE_TYPE_STATUS2);
	scr_addintconst(L, MESSAGE_TYPE_STATUS3);
	scr_addintconst(L, MESSAGE_TYPE_BRIGHT1);
	scr_addintconst(L, MESSAGE_TYPE_BRIGHT2);
	scr_addintconst(L, MESSAGE_TYPE_BRIGHT3);
	scr_addintconst(L, MESSAGE_TYPE_ANNOUNCE);
	scr_addintconst(L, MESSAGE_TYPE_BIGANNOUNCE);
	scr_addintconst(L, MESSAGE_TYPE_SMALLANNOUNCE);

	scr_addintconst(L, CLIENT_STATE_INITIAL);
	scr_addintconst(L, CLIENT_STATE_MOTD);
	scr_addintconst(L, CLIENT_STATE_INGAME);

	scr_addintconst(L, ENTITY_PROP_ROT_X);
	scr_addintconst(L, ENTITY_PROP_ROT_Y);
	scr_addintconst(L, ENTITY_PROP_ROT_Z);

	scr_addintconst(L, CPE_VELCTL_ADDALL);
	scr_addintconst(L, CPE_VELCTL_SETX);
	scr_addintconst(L, CPE_VELCTL_SETY);
	scr_addintconst(L, CPE_VELCTL_SETZ);
	scr_addintconst(L, CPE_VELCTL_SETALL);

	scr_newlib(L, clientlib);
	*(void **)scr_allocmem(L, sizeof(Client *)) = CLIENT_BROADCAST;
	scr_setmemtype(L, CSSCRIPTS_MCLIENT);
	scr_settabfield(L, -2, "broadcast");

	return 1;
}
