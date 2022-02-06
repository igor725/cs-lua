#include <core.h>
#include <list.h>
#include <log.h>
#include <str.h>
#include <event.h>
#include <command.h>
#include <platform.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luaworld.h"

AListField *headPlugin = NULL;

Plugin_SetVersion(1)

static void callallclient(Client *client, cs_str func) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, func)) {
			lua_pushclient(plugin->L, client);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static void evthandshake(void *param) {
	callallclient(param, "onHandshake");
}

static void evtdisconnect(void *param) {
	callallclient(param, "onDisconnect");
}

static void evtworldadded(void *param) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onWorldAdded")) {
			lua_pushworld(plugin->L, param);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static void evtmove(void *param) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onMove")) {
			lua_pushclient(plugin->L, param);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static void evtrotate(void *param) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onRotate")) {
			lua_pushclient(plugin->L, param);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static void evtheldchange(void *param) {
	onHeldBlockChange *a = (onHeldBlockChange *)param;
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onHeldBlockChange")) {
			lua_pushclient(plugin->L, a->client);
			lua_pushinteger(plugin->L, a->curr);
			lua_pushinteger(plugin->L, a->prev);
			LuaPlugin_Call(plugin, 3, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static cs_bool evtonmessage(void *param) {
	onMessage *a = (onMessage *)param;
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		LuaPlugin_Lock(plugin);
		cs_bool ret = true;
		if(LuaPlugin_GlobalLookup(plugin, "onMessage")) {
			lua_pushclient(plugin->L, a->client);
			lua_pushnumber(plugin->L, a->type);
			lua_pushstring(plugin->L, a->message);
			if(LuaPlugin_Call(plugin, 3, 2)) {
				if(lua_isboolean(plugin->L, -2)) {
					ret = (cs_bool)lua_toboolean(plugin->L, -2);
				} else {
					if(!lua_isnil(plugin->L, -2))
						a->type = (cs_byte)luaL_checkint(plugin->L, -2);
					if(!lua_isnil(plugin->L, -1))
						a->message = (cs_char *)luaL_checkstring(plugin->L, -1);
				}
			}
		}
		LuaPlugin_Unlock(plugin);
		if(!ret) return ret;
	}

	return true;
}

static LuaPlugin *getplugin(cs_str name) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		if(String_Compare(plugin->scrname, name)) return plugin;
	}

	return NULL;
}

COMMAND_FUNC(Lua) {
	COMMAND_SETUSAGE("/lua <list/load/unload/reload> [scriptname]");

	cs_char subcmd[64], plname[64];
	if(COMMAND_GETARG(subcmd, 64, 0)) {
		if(String_CaselessCompare(subcmd, "list")) {
			COMMAND_PRINT("Work in progress");
		} else {
			if(!COMMAND_GETARG(plname, 64, 1)) {
				COMMAND_PRINTUSAGE;
			} else if(!String_FindSubstr(plname, ".lua")) {
				String_Append(plname, 64, ".lua");
			}

			LuaPlugin *plugin = getplugin(plname);

			if(String_CaselessCompare(subcmd, "load")) {
				if(plugin) {
					COMMAND_PRINT("This script is already loaded");
				} else {
					COMMAND_PRINT("Work in progress");
				}
			} else {
				if(!plugin) {
					COMMAND_PRINTF("Script %s not found", plname);
				}

				if(String_CaselessCompare(subcmd, "unload")) {
					LuaPlugin_Lock(plugin);
					if(LuaPlugin_GlobalLookup(plugin, "onStop")) {
						lua_pushboolean(plugin->L, 0);
						if(!LuaPlugin_Call(plugin, 1, 1) || (!lua_isnil(plugin->L, -1) && !lua_toboolean(plugin->L, -1))) {
							LuaPlugin_Unlock(plugin);
							COMMAND_PRINT("This script cannot be unloaded right now");
						}
					}
					plugin->unloaded = true;
					LuaPlugin_Unlock(plugin);
					COMMAND_PRINTF("Script unloaded successfully", plname);
				} else if(String_CaselessCompare(subcmd, "reload")) {
					if(!plugin->hotreload) {
						COMMAND_PRINT("Hot reload was disabled for this script");
					}

					if(LuaPlugin_Reload(plugin)) {
						COMMAND_PRINT("Script reloaded successfully");
					} else {
						COMMAND_PRINT("This script cannot be reloaded right now");
					}
				}
			}
		}
	}

	COMMAND_PRINTUSAGE;
}

static void evttick(void *param) {
	(void)param;
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		if(plugin->unloaded) {
			AList_Remove(&headPlugin, tmp);
			LuaPlugin_Close(plugin);
			break;
		}
	}
}

EventRegBunch events[] = {
	{'v', EVT_ONHANDSHAKEDONE, evthandshake},
	{'v', EVT_ONDISCONNECT, evtdisconnect},
	{'v', EVT_WORLDADDED, evtworldadded},
	{'v', EVT_ONMOVE, evtmove},
	{'v', EVT_ONROTATE, evtrotate},
	{'v', EVT_ONHELDBLOCKCHNG, evtheldchange},
	{'b', EVT_ONMESSAGE, evtonmessage},
	{'v', EVT_ONTICK, evttick},
	{0, 0, NULL}
};

cs_bool Plugin_Load(void) {
	DirIter sIter;
	Directory_Ensure("scripts");
	if(Iter_Init(&sIter, "scripts", "lua")) {
		do {
			if(sIter.isDir || !sIter.cfile) continue;
			LuaPlugin *plugin = LuaPlugin_Open(sIter.cfile);
			if(!plugin) {
				Log_Error("Failed to load plugin %s", sIter.cfile);
				continue;
			}
			AList_AddField(&headPlugin, plugin);
		} while(Iter_Next(&sIter));
	}
	Iter_Close(&sIter);

	COMMAND_ADD(Lua, CMDF_OP, "Lua manager");
	return Event_RegisterBunch(events);
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	COMMAND_REMOVE(Lua);
	Event_UnregisterBunch(events);

	AListField *tmp;
	while((tmp = headPlugin) != NULL) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(tmp).ptr;
		LuaPlugin_Lock(plugin);
		AList_Remove(&headPlugin, tmp);
		if(LuaPlugin_GlobalLookup(plugin, "onStop")) {
			lua_pushboolean(plugin->L, 1);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
		LuaPlugin_Close(plugin);
	}
	return true;
}
