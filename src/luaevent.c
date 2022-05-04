#include <core.h>
#include <event.h>
#include <list.h>
#include <server.h>
#include <client.h>
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

static void evthandshake(void *param) {
	onHandshakeDone *a = (onHandshakeDone *)param;
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onHandshake")) {
			lua_pushclient(script->L, a->client);
			if(LuaScript_Call(script, 1, 1)) {
				if(luaL_testudata(script->L, -1, CSLUA_MWORLD)) {
					a->world = lua_checkworld(script->L, -1);
					LuaScript_Unlock(script);
					break;
				}
			}
		}
		LuaScript_Unlock(script);
	}
}

static cs_bool evtconnect(void *param) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onConnect")) {
			lua_pushclient(script->L, param);
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

static void evtdisconnect(void *param) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onDisconnect")) {
			lua_pushclient(script->L, param);
			lua_pushstring(script->L, Client_GetDisconnectReason(param));
			LuaScript_Call(script, 2, 0);
		}
		lua_clearcuboids(script->L, param);
		lua_clearclient(script->L, param);
		LuaScript_Unlock(script);
	}
}

static void evtusertype(void *param) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onUserTypeChange")) {
			lua_pushclient(script->L, param);
			LuaScript_Call(script, 1, 0);
		}
		LuaScript_Unlock(script);
	}
}

static void evtonspawn(void *param) {
	onSpawn *a = (onSpawn *)param;
	callallclient(a->client, "onSpawn");
}

static void evtondespawn(void *param) {
	callallclient(param, "onDespawn");
}

static void evtonclick(void *param) {
	onPlayerClick *a = (onPlayerClick *)param;
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onPlayerClick")) {
			lua_pushclient(script->L, a->client);
			lua_createtable(script->L, 0, 6);
			lua_pushinteger(script->L, (lua_Integer)a->button);
			lua_setfield(script->L, -2, "button");
			lua_pushboolean(script->L, a->action == 0);
			lua_setfield(script->L, -2, "action");
			lua_pushclient(script->L, Client_GetByID(a->tgid));
			lua_setfield(script->L, -2, "target");
			*lua_newangle(script->L) = a->angle;
			lua_setfield(script->L, -2, "angle");
			LuaVector *vec = lua_newvector(script->L);
			vec->type = LUAVECTOR_TSHORT;
			vec->value.s = a->tgpos;
			lua_setfield(script->L, -2, "position");
			lua_pushinteger(script->L, (lua_Integer)a->tgface);
			lua_setfield(script->L, -2, "face");
			LuaScript_Call(script, 2, 0);
		}
		LuaScript_Unlock(script);
	}
}

static cs_bool evtonblockplace(void *param) {
	onBlockPlace *a = (onBlockPlace *)param;
	AListField *tmp;

	cs_str func = NULL;
	switch(a->mode) {
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
			lua_pushclient(script->L, a->client);
			LuaVector *vec = lua_newvector(script->L);
			vec->value.s = a->pos;
			vec->type = 1;
			lua_pushinteger(script->L, a->id);
			if(LuaScript_Call(script, 3, 1)) {
				cs_bool succ = true;
				if(lua_isnumber(script->L, -1))
					a->id = (BlockID)lua_tointeger(script->L, -1);
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

static void evtworldadded(void *param) {
	callallworld(param, "onWorldAdded");
}

static void evtworldremoved(void *param) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onWorldRemoved")) {
			lua_pushworld(script->L, param);
			LuaScript_Call(script, 1, 0);
		}
		lua_clearworld(script->L, param);
		LuaScript_Unlock(script);
	}
}

static void evtworldloaded(void *param) {
	callallworld(param, "onWorldLoaded");
}

static void evtworldunloaded(void *param) {
	callallworld(param, "onWorldUnloaded");
}

static void evtonweather(void *param) {
	callallworld(param, "onWeatherChange");
}

static void evtoncolor(void *param) {
	callallworld(param, "onColorChange");
}

static void evtmove(void *param) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onMove")) {
			lua_pushclient(script->L, param);
			LuaScript_Call(script, 1, 0);
		}
		LuaScript_Unlock(script);
	}
}

static void evtrotate(void *param) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onRotate")) {
			lua_pushclient(script->L, param);
			LuaScript_Call(script, 1, 0);
		}
		LuaScript_Unlock(script);
	}
}

static void evtheldchange(void *param) {
	onHeldBlockChange *a = (onHeldBlockChange *)param;
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onHeldBlockChange")) {
			lua_pushclient(script->L, a->client);
			lua_pushinteger(script->L, a->curr);
			lua_pushinteger(script->L, a->prev);
			LuaScript_Call(script, 3, 0);
		}
		LuaScript_Unlock(script);
	}
}

static cs_bool evtonmessage(void *param) {
	onMessage *a = (onMessage *)param;
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		cs_bool ret = true;
		if(LuaScript_GlobalLookup(script, "onMessage")) {
			lua_pushclient(script->L, a->client);
			lua_pushnumber(script->L, a->type);
			lua_pushstring(script->L, a->message);
			if(LuaScript_Call(script, 3, 2)) {
				if(lua_isboolean(script->L, -2)) {
					ret = (cs_bool)lua_toboolean(script->L, -2);
				} else {
					if(!lua_isnil(script->L, -2))
						a->type = (cs_byte)luaL_checkinteger(script->L, -2);
					if(!lua_isnil(script->L, -1))
						a->message = (cs_char *)luaL_checkstring(script->L, -1);
				}
				lua_pop(script->L, 2);
			} else ret = false;
		}
		LuaScript_Unlock(script);
		if(!ret) return ret;
	}

	return true;
}

static void evttick(void *param) {
	(void)param;
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
				lua_pushinteger(script->L, (lua_Integer)*(cs_int32 *)param);
				LuaScript_Call(script, 1, 0);
			}
			LuaScript_Unlock(script);
		}
	}
}

static void evtpluginmsg(void *param) {
	onPluginMessage *a = (onPluginMessage *)param;

	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onPluginMessage")) {
			lua_pushclient(script->L, a->client);
			lua_pushinteger(script->L, (lua_Integer)a->channel);
			lua_pushlstring(script->L, a->message, 64);
			LuaScript_Call(script, 3, 0);
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
