#include <core.h>
#include <event.h>
#include <server.h>
#include <client.h>
#include <command.h>
#include <platform.h>
#include "luamain.h"
#include "luascript.h"
#include "luaangle.h"
#include "luavector.h"
#include "luaworld.h"
#include "luaclient.h"
#include "luacuboid.h"

static void evtpoststart(void *param) {
	(void)param;

	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "postStart"))
			Script_Call(script, 0, 0);
		Script_Unlock(script);
	})
}

static void callallclient(Client *client, cs_str func) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, func)) {
			scr_pushclient(script->L, client);
			Script_Call(script, 1, 0);
		}
		Script_Unlock(script);
	})
}

static void callallworld(World *world, cs_str func) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, func)) {
			scr_pushworld(script->L, world);
			Script_Call(script, 1, 0);
		}
		Script_Unlock(script);
	})
}

static void evthandshake(onHandshakeDone *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onHandshake")) {
			scr_pushclient(script->L, obj->client);
			if(Script_Call(script, 1, 1)) {
				if(scr_testmemtype(script->L, -1, CSSCRIPTS_MWORLD)) {
					obj->world = scr_toworld(script->L, -1);
					Script_Unlock(script);
					break;
				}
			}
		}
		Script_Unlock(script);
	})
}

static cs_bool evtconnect(Client *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onConnect")) {
			scr_pushclient(script->L, obj);
			if(Script_Call(script, 1, 1)) {
				cs_bool succ = (cs_bool)scr_isnull(script->L, -1) ||
				scr_toboolean(script->L, -1);
				scr_stackpop(script->L, 1);
				if(!succ) {
					Script_Unlock(script);
					return false;
				}
			} else {
				Script_Unlock(script);
				return false;
			}
		}
		Script_Unlock(script);
	})

	return true;
}

static void evtdisconnect(Client *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onDisconnect")) {
			scr_pushclient(script->L, obj);
			scr_pushstring(script->L, Client_GetDisconnectReason(obj));
			Script_Call(script, 2, 0);
		}
		scr_clearcuboids(script->L, obj);
		/**
		 * WARN: После вызова функции scr_clearclient
		 * не должно происходить никаких вызовов
		 * scr_pushclient для данного поинтера!
		 * 
		 */
		scr_clearclient(script->L, obj);
		Script_Unlock(script);
	})
}

static void evtusertype(Client *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onUserTypeChange")) {
			scr_pushclient(script->L, obj);
			Script_Call(script, 1, 0);
		}
		Script_Unlock(script);
	})
}

static void evtonspawn(onSpawn *obj) {
	callallclient(obj->client, "onSpawn");
}

static void evtondespawn(Client *obj) {
	callallclient(obj, "onDespawn");
}

static void evtonclick(onPlayerClick *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onPlayerClick")) {
			scr_pushclient(script->L, obj->client);
			scr_newntable(script->L, 0, 6);
			scr_pushinteger(script->L, (scr_Integer)obj->button);
			scr_settabfield(script->L, -2, "button");
			scr_pushboolean(script->L, obj->action == 0);
			scr_settabfield(script->L, -2, "action");
			scr_pushclient(script->L, Client_GetByID(obj->tgid));
			scr_settabfield(script->L, -2, "target");
			*scr_newangle(script->L) = obj->angle;
			scr_settabfield(script->L, -2, "angle");
			LuaVector *vec = scr_newvector(script->L);
			vec->type = LUAVECTOR_TSHORT;
			vec->value.s = obj->tgpos;
			scr_settabfield(script->L, -2, "position");
			scr_pushinteger(script->L, (scr_Integer)obj->tgface);
			scr_settabfield(script->L, -2, "face");
			Script_Call(script, 2, 0);
		}
		Script_Unlock(script);
	})
}

static cs_bool evtonblockplace(onBlockPlace *obj) {
	cs_str func = NULL;
	switch(obj->mode) {
		case SETBLOCK_MODE_CREATE:
			func = "onBlockPlace";
			break;
		case SETBLOCK_MODE_DESTROY:
			func = "onBlockDestroy";
			break;
		default:
			return true;
	}

	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, func)) {
			scr_pushclient(script->L, obj->client);
			LuaVector *vec = scr_newvector(script->L);
			vec->type = LUAVECTOR_TSHORT;
			vec->value.s = obj->pos;
			scr_pushinteger(script->L, obj->id);
			if(Script_Call(script, 3, 1)) {
				cs_bool succ = true;
				if(scr_isnumber(script->L, -1))
					obj->id = (BlockID)scr_tointeger(script->L, -1);
				else if(scr_isboolean(script->L, -1))
					succ = scr_toboolean(script->L, -1);
				scr_stackpop(script->L, 1);
				if(!succ) {
					Script_Unlock(script);
					return false;
				}
			} else {
				Script_Unlock(script);
				return false;
			}
		}
		Script_Unlock(script);
	})

	return true;
}

static void evtworldadded(World *obj) {
	callallworld(obj, "onWorldAdded");
}

static void evtworldremoved(World *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onWorldRemoved")) {
			scr_pushworld(script->L, obj);
			Script_Call(script, 1, 0);
		}
		scr_clearworld(script->L, obj);
		Script_Unlock(script);
	})
}

static void evtworldstatus(World *obj) {
	callallworld(obj, "onWorldStatusChanged");
}

static cs_bool evtpreworldenvupdate(preWorldEnvUpdate *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "preWorldEnvUpdate")) {
			scr_pushworld(script->L, obj->world);
			scr_pushinteger(script->L, obj->values);
			scr_pushinteger(script->L, obj->props);
			scr_pushinteger(script->L, obj->colors);
			if(Script_Call(script, 4, 1)) {
				if(!scr_isnull(script->L, -1)) {
					cs_bool ret = scr_toboolean(script->L, -1);
					scr_stackpop(script->L, 1);
					return ret;
				}
			} else {
				Script_Unlock(script);
				return false;
			}
		}
		Script_Unlock(script);
	})

	return true;
}

static void evtmove(Client *obj) {
	callallclient(obj, "onMove");
}

static void evtrotate(Client *obj) {
	callallclient(obj, "onRotate");
}

static void evtheldchange(onHeldBlockChange *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onHeldBlockChange")) {
			scr_pushclient(script->L, obj->client);
			scr_pushinteger(script->L, obj->curr);
			scr_pushinteger(script->L, obj->prev);
			Script_Call(script, 3, 0);
		}
		Script_Unlock(script);
	})
}

static cs_bool evtonmessage(onMessage *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		cs_bool ret = true;
		if(Script_GlobalLookup(script, "onMessage")) {
			scr_pushclient(script->L, obj->client);
			scr_pushnumber(script->L, obj->type);
			scr_pushstring(script->L, obj->message);
			if(Script_Call(script, 3, 2)) {
				if(scr_isboolean(script->L, -2)) {
					ret = scr_toboolean(script->L, -2);
				} else {
					if(scr_isnumber(script->L, -2))
						obj->type = (cs_byte)scr_tointeger(script->L, -2);
					if(scr_isstring(script->L, -1))
						String_Copy(obj->message, sizeof(obj->message), scr_tostring(script->L, -1));
				}
				scr_stackpop(script->L, 2);
			} else ret = false;
		}
		Script_Unlock(script);
		if(!ret) return ret;
	})

	return true;
}

static void evttick(cs_int32 *obj) {
	ScriptList_Iter({
		if(script->unloaded) {
			ScriptList_RemoveCurrent();
			Script_Close(script);
			break;
		} else {
			Script_Lock(script);
			if(Script_GlobalLookup(script, "onTick")) {
				scr_pushinteger(script->L, (scr_Integer)*obj);
				Script_Call(script, 1, 0);
			}
			Script_Unlock(script);
		}
	})
}

static void evtpluginmsg(onPluginMessage *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "onPluginMessage")) {
			scr_pushclient(script->L, obj->client);
			scr_pushinteger(script->L, (scr_Integer)obj->channel);
			scr_pushlstring(script->L, obj->message, 64);
			Script_Call(script, 3, 0);
		}
		Script_Unlock(script);
	})
}

static void evtprecommand(preCommand *obj) {
	ScriptList_Iter({
		if(script != Command_GetUserData(obj->command)) continue;
		Script_Lock(script);
		if(Script_GlobalLookup(script, "preCommand")) {
			scr_pushstring(script->L, Command_GetName(obj->command));
			scr_pushclient(script->L, obj->caller);
			scr_pushstring(script->L, obj->args);
			scr_pushboolean(script->L, obj->allowed);
			if(Script_Call(script, 4, 1)) {
				if(scr_isboolean(script->L, -1))
					obj->allowed = scr_toboolean(script->L, -1);
				scr_stackpop(script->L, 1);
			}
		}
		Script_Unlock(script);
	})
}

static void evtprehandshakedone(preHandshakeDone *obj) {
	ScriptList_Iter({
		Script_Lock(script);
		if(Script_GlobalLookup(script, "preHandshakeDone")) {
			scr_pushclient(script->L, obj->client);
			scr_pushstring(script->L, obj->name);
			scr_pushstring(script->L, obj->motd);
			if(Script_Call(script, 3, 2)) {
				if(scr_isstring(script->L, -1))
					String_Copy(obj->motd, sizeof(obj->motd), scr_tostring(script->L, -1));
				if(scr_isstring(script->L, -2))
					String_Copy(obj->name, sizeof(obj->name), scr_tostring(script->L, -2));
				scr_stackpop(script->L, 2);
			}
		}
		Script_Unlock(script);
	})
}

Event_DeclareBunch (events) {
	EVENT_BUNCH_ADD('v', EVT_POSTSTART, evtpoststart),
	EVENT_BUNCH_ADD('v', EVT_ONTICK, evttick),
	EVENT_BUNCH_ADD('b', EVT_ONCONNECT, evtconnect),
	EVENT_BUNCH_ADD('v', EVT_ONHANDSHAKEDONE, evthandshake),
	EVENT_BUNCH_ADD('v', EVT_ONUSERTYPECHANGE, evtusertype),
	EVENT_BUNCH_ADD('v', EVT_ONDISCONNECT, evtdisconnect),
	EVENT_BUNCH_ADD('v', EVT_ONSPAWN, evtonspawn),
	EVENT_BUNCH_ADD('v', EVT_ONDESPAWN, evtondespawn),
	EVENT_BUNCH_ADD('b', EVT_ONMESSAGE, evtonmessage),
	EVENT_BUNCH_ADD('v', EVT_ONHELDBLOCKCHNG, evtheldchange),
	EVENT_BUNCH_ADD('b', EVT_ONBLOCKPLACE, evtonblockplace),
	EVENT_BUNCH_ADD('v', EVT_ONCLICK, evtonclick),
	EVENT_BUNCH_ADD('v', EVT_ONMOVE, evtmove),
	EVENT_BUNCH_ADD('v', EVT_ONROTATE, evtrotate),
	EVENT_BUNCH_ADD('v', EVT_ONWORLDSTATUSCHANGE, evtworldstatus),
	EVENT_BUNCH_ADD('v', EVT_ONWORLDADDED, evtworldadded),
	EVENT_BUNCH_ADD('v', EVT_ONWORLDREMOVED, evtworldremoved),
	EVENT_BUNCH_ADD('v', EVT_ONPLUGINMESSAGE, evtpluginmsg),
	EVENT_BUNCH_ADD('v', EVT_PRECOMMAND, evtprecommand),
	EVENT_BUNCH_ADD('v', EVT_PREHANDSHAKEDONE, evtprehandshakedone),
	EVENT_BUNCH_ADD('b', EVT_PREWORLDENVUPDATE, evtpreworldenvupdate),

	EVENT_BUNCH_END
};

cs_bool LuaEvent_Register(void) {
	// Вызываем postStart если в момент загрузки
	// плагина сервер уже работал.
	if(Server_Ready) evtpoststart(NULL);
	return Event_RegisterBunch(events);
}

void LuaEvent_Unregister(void) {
	Event_UnregisterBunch(events);
}
