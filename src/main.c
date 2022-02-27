#include <core.h>
#include <list.h>
#include <log.h>
#include <str.h>
#include <event.h>
#include <command.h>
#include <platform.h>
#include <plugin.h>
#include "luascript.h"
#include "luaclient.h"
#include "luaworld.h"
#include "luavector.h"
#include "luaangle.h"
#include "cs-survival/src/survitf.h"

AListField *headScript = NULL;

Plugin_SetVersion(1);

INL static LuaScript *getscriptptr(AListField *field) {
	return (LuaScript *)AList_GetValue(field).ptr;
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

// TODO: Дать скриптам возможность менять параметр world
static void evthandshake(void *param) {
	onHandshakeDone *a = (onHandshakeDone *)param;
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, "onHandshake")) {
			lua_pushclient(script->L, a->client);
			if(LuaScript_Call(script, 1, 1)) {
				if(luaL_testudata(script->L, -1, "World")) {
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
			lua_newtable(script->L);
			lua_pushinteger(script->L, (lua_Integer)a->button);
			lua_setfield(script->L, -2, "button");
			lua_pushinteger(script->L, (lua_Integer)a->action);
			lua_setfield(script->L, -2, "action");
			lua_pushclient(script->L, Client_GetByID(a->tgid));
			lua_setfield(script->L, -2, "target");
			Ang *ang = lua_newangle(script->L);
			*ang = a->angle;
			lua_setfield(script->L, -2, "angle");
			LuaVector *vec = lua_newvector(script->L);
			vec->value.s = a->tgpos;
			vec->type = 1;
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

	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		LuaScript_Lock(script);
		if(LuaScript_GlobalLookup(script, a->mode ? "onBlockPlace" : "onBlockDestroy")) {
			lua_pushclient(script->L, a->client);
			LuaVector *vec = lua_newvector(script->L);
			vec->value.s = a->pos;
			vec->type = 1;
			lua_pushinteger(script->L, a->id);
			if(LuaScript_Call(script, 3, 1)) {
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

static LuaScript *getscript(cs_str name) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		if(String_Compare(script->scrname, name)) return script;
	}

	return NULL;
}

COMMAND_FUNC(Lua) {
	COMMAND_SETUSAGE("/lua <list/load/unload/reload/disable/enable> [scriptname]");

	cs_char subcmd[64], plname[64];
	if(COMMAND_GETARG(subcmd, 64, 0)) {
		if(String_CaselessCompare(subcmd, "list")) {
			COMMAND_PRINTLINE("&eList of loaded Lua scripts:");
			AListField *tmp;
			cs_uint32 count = 0;
			List_Iter(tmp, headScript) {
				LuaScript *script = getscriptptr(tmp);
				LuaScript_Lock(script);
				int usage = lua_gc(script->L, LUA_GCCOUNT, 0);
				COMMAND_PRINTFLINE("%d. &9%s&f, &a%dKb&f used", ++count, script->scrname, usage);
				LuaScript_Unlock(script);
			}
			return false;
		} else {
			if(!COMMAND_GETARG(plname, 64, 1)) {
				COMMAND_PRINTUSAGE;
			} else if(!String_FindSubstr(plname, ".lua")) {
				String_Append(plname, 64, ".lua");
			}

			LuaScript *script = getscript(plname);

			if(String_CaselessCompare(subcmd, "load")) {
				if(script) {
					COMMAND_PRINT("&cThis script is already loaded");
				}

				script = LuaScript_Open(plname);
				if(!script) {
					COMMAND_PRINT("&cFailed to load specified script");
				}

				if(AList_AddField(&headScript, script)) {
					COMMAND_PRINTF("&aScript \"%s\" loaded successfully", plname);
				} else {
					LuaScript_Close(script);
					COMMAND_PRINT("&cUnexpected error");
				}
			} else if(String_CaselessCompare(subcmd, "enable")) {
				COMMAND_PRINT("&7Work in progress");
			} else if(String_CaselessCompare(subcmd, "disable")) {
				COMMAND_PRINT("&7Work in progress");
			} else {
				if(!script) {
					COMMAND_PRINTF("&cScript \"%s\" not found", plname);
				}

				if(String_CaselessCompare(subcmd, "unload")) {
					LuaScript_Lock(script);
					if(LuaScript_GlobalLookup(script, "onStop")) {
						lua_pushboolean(script->L, 0);
						if(!LuaScript_Call(script, 1, 1)) {
							unlerr:
							LuaScript_Unlock(script);
							COMMAND_PRINT("&cThis script cannot be unloaded right now");
						}
						if(!lua_isnil(script->L, -1) && !lua_toboolean(script->L, -1)) {
							lua_pop(script->L, 1);
							goto unlerr;
						}
					}
					script->unloaded = true;
					LuaScript_Unlock(script);
					COMMAND_PRINTF("&aScript unloaded successfully", plname);
				} else if(String_CaselessCompare(subcmd, "reload")) {
					if(!script->hotreload) {
						COMMAND_PRINT("&cThis script does not support hot reloading");
					}

					if(LuaScript_Reload(script)) {
						COMMAND_PRINT("&aScript reloaded successfully");
					} else {
						COMMAND_PRINT("&cThis script cannot be reloaded right now");
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

EventRegBunch events[] = {
	{'v', EVT_POSTSTART, (void *)evtpoststart},
	{'v', EVT_ONHANDSHAKEDONE, (void *)evthandshake},
	{'b', EVT_ONCONNECT, (void *)evtconnect},
	{'v', EVT_ONDISCONNECT, (void *)evtdisconnect},
	{'v', EVT_ONUSERTYPECHANGE, (void *)evtusertype},
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
	Directory_Ensure("scripts" PATH_DELIM "disabled"); // Сюда будут переноситься выключенные скрипты
	if(Iter_Init(&sIter, "scripts", "lua")) {
		do {
			if(sIter.isDir || !sIter.cfile) continue;
			LuaScript *script = LuaScript_Open(sIter.cfile);
			if(!script) {
				Log_Error("Failed to load script \"%s\"", sIter.cfile);
				continue;
			}
			AList_AddField(&headScript, script);
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

	while(headScript) {
		LuaScript *script = (LuaScript *)AList_GetValue(headScript).ptr;
		LuaScript_Lock(script);
		AList_Remove(&headScript, headScript);
		if(LuaScript_GlobalLookup(script, "onStop")) {
			lua_pushboolean(script->L, 1);
			LuaScript_Call(script, 1, 0);
		}
		LuaScript_Unlock(script);
		LuaScript_Close(script);
	}

	return true;
}
