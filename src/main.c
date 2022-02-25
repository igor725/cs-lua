#include <core.h>
#include <list.h>
#include <log.h>
#include <str.h>
#include <event.h>
#include <command.h>
#include <platform.h>
#include <plugin.h>
#include "luaplugin.h"
#include "luaclient.h"
#include "luaworld.h"
#include "luavector.h"
#include "luaangle.h"
#include "cs-survival/src/survitf.h"

AListField *headPlugin = NULL;

Plugin_SetVersion(1);

INL static LuaPlugin *getpluginptr(AListField *field) {
	return (LuaPlugin *)AList_GetValue(field).ptr;
}

static void callallclient(Client *client, cs_str func) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, func)) {
			lua_pushclient(plugin->L, client);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static void callallworld(World *world, cs_str func) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, func)) {
			lua_pushworld(plugin->L, world);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

// TODO: Дать скриптам возможность менять параметр world
static void evthandshake(void *param) {
	onHandshakeDone *a = (onHandshakeDone *)param;
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onHandshake")) {
			lua_pushclient(plugin->L, a->client);
			if(LuaPlugin_Call(plugin, 1, 1)) {
				if(luaL_testudata(plugin->L, -1, "World")) {
					a->world = lua_checkworld(plugin->L, -1);
					LuaPlugin_Unlock(plugin);
					break;
				}
			}
		}
		LuaPlugin_Unlock(plugin);
	}
}

static void evtdisconnect(void *param) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onDisconnect")) {
			lua_pushclient(plugin->L, param);
			lua_pushstring(plugin->L, Client_GetDisconnectReason(param));
			LuaPlugin_Call(plugin, 2, 0);
		}
		lua_clearclient(plugin->L, param);
		LuaPlugin_Unlock(plugin);
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
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onPlayerClick")) {
			lua_pushclient(plugin->L, a->client);
			lua_newtable(plugin->L);
			lua_pushinteger(plugin->L, (lua_Integer)a->button);
			lua_setfield(plugin->L, -2, "button");
			lua_pushinteger(plugin->L, (lua_Integer)a->action);
			lua_setfield(plugin->L, -2, "action");
			lua_pushclient(plugin->L, Client_GetByID(a->tgid));
			lua_setfield(plugin->L, -2, "target");
			Ang *ang = lua_newangle(plugin->L);
			*ang = a->angle;
			lua_setfield(plugin->L, -2, "angle");
			LuaVector *vec = lua_newvector(plugin->L);
			vec->value.s = a->tgpos;
			vec->type = 1;
			lua_setfield(plugin->L, -2, "position");
			lua_pushinteger(plugin->L, (lua_Integer)a->tgface);
			lua_setfield(plugin->L, -2, "face");
			LuaPlugin_Call(plugin, 2, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static cs_bool evtonblockplace(void *param) {
	onBlockPlace *a = (onBlockPlace *)param;
	AListField *tmp;

	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, a->mode ? "onBlockPlace" : "onBlockDestroy")) {
			lua_pushclient(plugin->L, a->client);
			LuaVector *vec = lua_newvector(plugin->L);
			vec->value.s = a->pos;
			vec->type = 1;
			lua_pushinteger(plugin->L, a->id);
			if(LuaPlugin_Call(plugin, 3, 1)) {
				if(!lua_isnil(plugin->L, -1) && !lua_toboolean(plugin->L, -1)) {
					lua_pop(plugin->L, 1);
					LuaPlugin_Unlock(plugin);
					return false;
				}
				lua_pop(plugin->L, 1);
			}
		}
		LuaPlugin_Unlock(plugin);
	}

	return true;
}

static void evtworldadded(void *param) {
	callallworld(param, "onWorldAdded");
}

static void evtworldremoved(void *param) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onWorldRemoved")) {
			lua_pushworld(plugin->L, param);
			LuaPlugin_Call(plugin, 1, 0);
		}
		lua_clearworld(plugin->L, param);
		LuaPlugin_Unlock(plugin);
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
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
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
		LuaPlugin *plugin = getpluginptr(tmp);
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
		LuaPlugin *plugin = getpluginptr(tmp);
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
		LuaPlugin *plugin = getpluginptr(tmp);
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
						a->type = (cs_byte)luaL_checkinteger(plugin->L, -2);
					if(!lua_isnil(plugin->L, -1))
						a->message = (cs_char *)luaL_checkstring(plugin->L, -1);
				}
				lua_pop(plugin->L, 2);
			}
		}
		LuaPlugin_Unlock(plugin);
		if(!ret) return ret;
	}

	return true;
}

static void evtpluginmsg(void *param) {
	onPluginMessage *a = (onPluginMessage *)param;

	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "onPluginMessage")) {
			lua_pushclient(plugin->L, a->client);
			lua_pushinteger(plugin->L, (lua_Integer)a->channel);
			lua_pushlstring(plugin->L, a->message, 64);
			LuaPlugin_Call(plugin, 3, 0);
		}
		LuaPlugin_Unlock(plugin);
	}
}

static LuaPlugin *getplugin(cs_str name) {
	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
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
				}

				plugin = LuaPlugin_Open(plname);
				if(!plugin) {
					COMMAND_PRINT("Failed to load specified script");
				}

				if(AList_AddField(&headPlugin, plugin)) {
					COMMAND_PRINTF("Script \"%s\" loaded successfully", plname);
				} else {
					LuaPlugin_Close(plugin);
					COMMAND_PRINT("Unexpected error");
				}
			} else {
				if(!plugin) {
					COMMAND_PRINTF("Script \"%s\" not found", plname);
				}

				if(String_CaselessCompare(subcmd, "unload")) {
					LuaPlugin_Lock(plugin);
					if(LuaPlugin_GlobalLookup(plugin, "onStop")) {
						lua_pushboolean(plugin->L, 0);
						if(!LuaPlugin_Call(plugin, 1, 1)) {
							unlerr:
							LuaPlugin_Unlock(plugin);
							COMMAND_PRINT("This script cannot be unloaded right now");
						}
						if(!lua_isnil(plugin->L, -1) && !lua_toboolean(plugin->L, -1)) {
							lua_pop(plugin->L, 1);
							goto unlerr;
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
		LuaPlugin *plugin = getpluginptr(tmp);
		if(plugin->unloaded) {
			AList_Remove(&headPlugin, tmp);
			LuaPlugin_Close(plugin);
			break;
		} else {
			LuaPlugin_Lock(plugin);
			if(LuaPlugin_GlobalLookup(plugin, "onTick")) {
				lua_pushinteger(plugin->L, (lua_Integer)*(cs_int32 *)param);
				LuaPlugin_Call(plugin, 1, 0);
			}
			LuaPlugin_Unlock(plugin);
		}
	}
}

static void evtpoststart(void *param) {
	(void)param;

	AListField *tmp;
	List_Iter(tmp, headPlugin) {
		LuaPlugin *plugin = getpluginptr(tmp);
		LuaPlugin_Lock(plugin);
		if(LuaPlugin_GlobalLookup(plugin, "postStart"))
			LuaPlugin_Call(plugin, 0, 0);
		LuaPlugin_Unlock(plugin);
	}
}

EventRegBunch events[] = {
	{'v', EVT_POSTSTART, (void *)evtpoststart},
	{'v', EVT_ONHANDSHAKEDONE, (void *)evthandshake},
	{'v', EVT_ONDISCONNECT, (void *)evtdisconnect},
	{'v', EVT_ONWORLDADDED, (void *)evtworldadded},
	{'v', EVT_ONWORLDREMOVED, (void *)evtworldremoved},
	{'v', EVT_ONWORLDLOADED, (void *)evtworldloaded},
	{'v', EVT_ONWORLDUNLOADED, (void *)evtworldunloaded},
	{'v', EVT_ONWEATHER, (void *)evtonweather},
	{'v', EVT_ONCOLOR, (void *)evtoncolor},
	{'v', EVT_ONMOVE, (void *)evtmove},
	{'v', EVT_ONROTATE, (void *)evtrotate},
	{'v', EVT_ONCLICK, (void *)evtonclick},
	{'b', EVT_ONBLOCKPLACE, (void *)evtonblockplace},
	{'v', EVT_ONHELDBLOCKCHNG, (void *)evtheldchange},
	{'b', EVT_ONMESSAGE, (void *)evtonmessage},
	{'v', EVT_ONSPAWN, (void *)evtonspawn},
	{'v', EVT_ONDESPAWN, (void *)evtondespawn},
	{'v', EVT_ONTICK, (void *)evttick},
	{'v', EVT_ONPLUGINMESSAGE, (void *)evtpluginmsg},

	{0, 0, NULL}
};

void *SurvInterface = NULL;

void Plugin_RecvInterface(cs_str name, void *ptr, cs_size size) {
	if(String_Compare(name, SURV_ITF_NAME))
		SurvInterface = size == sizeof(SurvItf) ? ptr : NULL;
}

cs_bool Plugin_Load(void) {
	DirIter sIter;
	Directory_Ensure("lua"); // Папка для библиотек, подключаемых скриптами
	Directory_Ensure("scripts"); // Сами скрипты, загружаются автоматически
	if(Iter_Init(&sIter, "scripts", "lua")) {
		do {
			if(sIter.isDir || !sIter.cfile) continue;
			LuaPlugin *plugin = LuaPlugin_Open(sIter.cfile);
			if(!plugin) {
				Log_Error("Failed to load script \"%s\"", sIter.cfile);
				continue;
			}
			AList_AddField(&headPlugin, plugin);
		} while(Iter_Next(&sIter));
	}
	Iter_Close(&sIter);

	COMMAND_ADD(Lua, CMDF_OP, "Lua scripts management");
	return Event_RegisterBunch(events);
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	COMMAND_REMOVE(Lua);
	Event_UnregisterBunch(events);

	while(headPlugin) {
		LuaPlugin *plugin = (LuaPlugin *)AList_GetValue(headPlugin).ptr;
		LuaPlugin_Lock(plugin);
		AList_Remove(&headPlugin, headPlugin);
		if(LuaPlugin_GlobalLookup(plugin, "onStop")) {
			lua_pushboolean(plugin->L, 1);
			LuaPlugin_Call(plugin, 1, 0);
		}
		LuaPlugin_Unlock(plugin);
		LuaPlugin_Close(plugin);
	}

	return true;
}
