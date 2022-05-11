#include <core.h>
#include <event.h>
#include <list.h>
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

	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "postStart"))
			LuaScript_Call(script, 0, 0);
		LuaScript_Unlock(script);
	}
}

static void callallclient(Client *client, cs_str func) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, func)) {
			lua_pushclient(script->L, client);
			LuaScript_Call(script, 1, 0);
		}
		LuaScript_Unlock(script);
	}
}

static void callallworld(World *world, cs_str func) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, func)) {
			lua_pushworld(script->L, world);
			LuaScript_Call(script, 1, 0);
		}
		LuaScript_Unlock(script);
	}
}

static void evthandshake(onHandshakeDone *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onHandshake")) {
			lua_pushclient(script->L, obj->client);
			if(LuaScript_Call(script, 1, 1)) {
				if(luaL_testudata(script->L, -1, CSLUA_MWORLD)) {
					obj->world = lua_toworld(script->L, -1);
					LuaScript_Unlock(script);
					break;
				}
			}
		}
		LuaScript_Unlock(script);
	}
}

static cs_bool evtconnect(Client *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onConnect")) {
			lua_pushclient(script->L, obj);
			if(LuaScript_Call(script, 1, 1)) {
				cs_bool succ = (cs_bool)lua_isnil(script->L, -1) ||
				(cs_bool)lua_toboolean(script->L, -1);
				lua_pop(script->L, 1);
				if(!succ) {
					LuaScript_Unlock(script);
					return false;
				}
			} else {
				LuaScript_Unlock(script);
				return false;
			}
		}
		LuaScript_Unlock(script);
	}

	return true;
}

static void evtdisconnect(Client *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onDisconnect")) {
			lua_pushclient(script->L, obj);
			lua_pushstring(script->L, Client_GetDisconnectReason(obj));
			LuaScript_Call(script, 2, 0);
		}
		lua_clearcuboids(script->L, obj);
		/**
		 * WARN: После вызова функции lua_clearclient
		 * не должно происходить никаких вызовов
		 * lua_pushclient для данного поинтера!
		 * 
		 */
		lua_clearclient(script->L, obj);
		LuaScript_Unlock(script);
	}
}

static void evtusertype(Client *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onUserTypeChange")) {
			lua_pushclient(script->L, obj);
			LuaScript_Call(script, 1, 0);
		}
		LuaScript_Unlock(script);
	}
}

static void evtonspawn(onSpawn *obj) {
	callallclient(obj->client, "onSpawn");
}

static void evtondespawn(Client *obj) {
	callallclient(obj, "onDespawn");
}

static void evtonclick(onPlayerClick *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onPlayerClick")) {
			lua_pushclient(script->L, obj->client);
			lua_createtable(script->L, 0, 6);
			lua_pushinteger(script->L, (lua_Integer)obj->button);
			lua_setfield(script->L, -2, "button");
			lua_pushboolean(script->L, obj->action == 0);
			lua_setfield(script->L, -2, "action");
			lua_pushclient(script->L, Client_GetByID(obj->tgid));
			lua_setfield(script->L, -2, "target");
			*lua_newangle(script->L) = obj->angle;
			lua_setfield(script->L, -2, "angle");
			LuaVector *vec = lua_newvector(script->L);
			vec->type = LUAVECTOR_TSHORT;
			vec->value.s = obj->tgpos;
			lua_setfield(script->L, -2, "position");
			lua_pushinteger(script->L, (lua_Integer)obj->tgface);
			lua_setfield(script->L, -2, "face");
			LuaScript_Call(script, 2, 0);
		}
		LuaScript_Unlock(script);
	}
}

static cs_bool evtonblockplace(onBlockPlace *obj) {
	AListField *tmp;

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

	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, func)) {
			lua_pushclient(script->L, obj->client);
			LuaVector *vec = lua_newvector(script->L);
			vec->type = LUAVECTOR_TSHORT;
			vec->value.s = obj->pos;
			lua_pushinteger(script->L, obj->id);
			if(LuaScript_Call(script, 3, 1)) {
				cs_bool succ = true;
				if(lua_isnumber(script->L, -1))
					obj->id = (BlockID)lua_tointeger(script->L, -1);
				else if(lua_isboolean(script->L, -1))
					succ = (cs_bool)lua_toboolean(script->L, -1);
				lua_pop(script->L, 1);
				if(!succ) {
					LuaScript_Unlock(script);
					return false;
				}
			} else {
				LuaScript_Unlock(script);
				return false;
			}
		}
		LuaScript_Unlock(script);
	}

	return true;
}

static void evtworldadded(World *obj) {
	callallworld(obj, "onWorldAdded");
}

static void evtworldremoved(World *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onWorldRemoved")) {
			lua_pushworld(script->L, obj);
			LuaScript_Call(script, 1, 0);
		}
		lua_clearworld(script->L, obj);
		LuaScript_Unlock(script);
	}
}

static void evtworldloaded(World *obj) {
	callallworld(obj, "onWorldLoaded");
}

static void evtworldunloaded(World *obj) {
	callallworld(obj, "onWorldUnloaded");
}

static void evtonweather(World *obj) {
	callallworld(obj, "onWeatherChange");
}

static void evtoncolor(World *obj) {
	callallworld(obj, "onColorChange");
}

static void evtmove(Client *obj) {
	callallclient(obj, "onMove");
}

static void evtrotate(Client *obj) {
	callallclient(obj, "onRotate");
}

static void evtheldchange(onHeldBlockChange *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onHeldBlockChange")) {
			lua_pushclient(script->L, obj->client);
			lua_pushinteger(script->L, obj->curr);
			lua_pushinteger(script->L, obj->prev);
			LuaScript_Call(script, 3, 0);
		}
		LuaScript_Unlock(script);
	}
}

static cs_bool evtonmessage(onMessage *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		cs_bool ret = true;
		if(LuaScript_GlobalLookup(script, "onMessage")) {
			lua_pushclient(script->L, obj->client);
			lua_pushnumber(script->L, obj->type);
			lua_pushstring(script->L, obj->message);
			if(LuaScript_Call(script, 3, 2)) {
				if(lua_isboolean(script->L, -2)) {
					ret = (cs_bool)lua_toboolean(script->L, -2);
				} else {
					if(lua_isnumber(script->L, -2))
						obj->type = (cs_byte)lua_tointeger(script->L, -2);
					if(lua_isstring(script->L, -1))
						obj->message = (cs_char *)lua_tostring(script->L, -1);
				}
				lua_pop(script->L, 2);
			} else ret = false;
		}
		LuaScript_Unlock(script);
		if(!ret) return ret;
	}

	return true;
}

static void evttick(cs_int32 *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		if(script->unloaded) {
			AList_Remove(&headScript, tmp);
			LuaScript_Close(script);
			break;
		} else {
			LuaScript_Lock(script);
			if(LuaScript_GlobalLookup(script, "onTick")) {
				lua_pushinteger(script->L, (lua_Integer)*obj);
				LuaScript_Call(script, 1, 0);
			}
			LuaScript_Unlock(script);
		}
	}
}

static void evtpluginmsg(onPluginMessage *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onPluginMessage")) {
			lua_pushclient(script->L, obj->client);
			lua_pushinteger(script->L, (lua_Integer)obj->channel);
			lua_pushlstring(script->L, obj->message, 64);
			LuaScript_Call(script, 3, 0);
		}
		LuaScript_Unlock(script);
	}
}

static void evtprecommand(preCommand *obj) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		if(script != Command_GetUserData(obj->command)) continue;
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "preCommand")) {
			lua_pushstring(script->L, Command_GetName(obj->command));
			lua_pushclient(script->L, obj->caller);
			lua_pushstring(script->L, obj->args);
			lua_pushboolean(script->L, obj->allowed);
			if(LuaScript_Call(script, 4, 1)) {
				if(lua_isboolean(script->L, -1))
					obj->allowed = (cs_bool)lua_toboolean(script->L, -1);
				lua_pop(script->L, 1);
			}
		}
		LuaScript_Unlock(script);
	}	
}

Event_DeclareBunch (events) {
	EVENT_BUNCH_ADD('v', EVT_POSTSTART, evtpoststart)
	EVENT_BUNCH_ADD('v', EVT_ONTICK, evttick)
	EVENT_BUNCH_ADD('b', EVT_ONCONNECT, evtconnect)
	EVENT_BUNCH_ADD('v', EVT_ONHANDSHAKEDONE, evthandshake)
	EVENT_BUNCH_ADD('v', EVT_ONUSERTYPECHANGE, evtusertype)
	EVENT_BUNCH_ADD('v', EVT_ONDISCONNECT, evtdisconnect)
	EVENT_BUNCH_ADD('v', EVT_ONSPAWN, evtonspawn)
	EVENT_BUNCH_ADD('v', EVT_ONDESPAWN, evtondespawn)
	EVENT_BUNCH_ADD('b', EVT_ONMESSAGE, evtonmessage)
	EVENT_BUNCH_ADD('v', EVT_ONHELDBLOCKCHNG, evtheldchange)
	EVENT_BUNCH_ADD('b', EVT_ONBLOCKPLACE, evtonblockplace)
	EVENT_BUNCH_ADD('v', EVT_ONCLICK, evtonclick)
	EVENT_BUNCH_ADD('v', EVT_ONMOVE, evtmove)
	EVENT_BUNCH_ADD('v', EVT_ONROTATE, evtrotate)
	EVENT_BUNCH_ADD('v', EVT_ONWEATHER, evtonweather)
	EVENT_BUNCH_ADD('v', EVT_ONCOLOR, evtoncolor)
	EVENT_BUNCH_ADD('v', EVT_ONWORLDADDED, evtworldadded)
	EVENT_BUNCH_ADD('v', EVT_ONWORLDLOADED, evtworldloaded)
	EVENT_BUNCH_ADD('v', EVT_ONWORLDREMOVED, evtworldremoved)
	EVENT_BUNCH_ADD('v', EVT_ONWORLDUNLOADED, evtworldunloaded)
	EVENT_BUNCH_ADD('v', EVT_ONPLUGINMESSAGE, evtpluginmsg)
	EVENT_BUNCH_ADD('v', EVT_PRECOMMAND, evtprecommand)

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
