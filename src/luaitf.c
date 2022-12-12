#include <core.h>
#include <plugin.h>
#include <str.h>

#include "luamain.h"
#include "luascript.h"
#include "luaitf.h"

LuaEventFunc callbacks[MAX_LUA_CALLBACKS] = {NULL};

static cs_bool addcallback(LuaEventFunc func) {
	for (cs_uint32 i = 0; i < MAX_LUA_CALLBACKS; i++) {
		if (!callbacks[i]) {
			callbacks[i] = func;
			return true;
		}
	}

	return false;
}

static cs_bool removecallback(LuaEventFunc func) {
	for (cs_uint32 i = 0; i < MAX_LUA_CALLBACKS; i++) {
		if (callbacks[i] == func) {
			callbacks[i] = NULL;
			return true;
		}
	}

	return false;
}

static void locklist(void) {
	Mutex_Lock(listMutex);
}

static void unlocklist(void) {
	Mutex_Unlock(listMutex);
}

static cs_bool runcommand(ELuaCommand cmd, cs_uint32 idx) {(void)idx;
	// TODO: Выполнялка команд
	switch (cmd) {
		case LUACOMMAND_RELOAD:
			break;
		case LUACOMMAND_UNLOAD:
			break;
	}

	return false;
}

static cs_uint32 reqscrinf(LuaInfo *li, cs_uint32 idx) {
	if (idx > 0 && idx < MAX_SCRIPTS_COUNT) {
		LuaScript *script = scripts[idx];
		li->name = String_AllocCopy(script->scrname);
		li->hotreload = script->hotreload;
		li->id = script->id;
		li->version = 1;
	} else return 0;

	for (cs_uint32 i = ++idx; i < MAX_SCRIPTS_COUNT; i++)
		if (scripts[idx]) return i;

	return 0;
}

static void disscrinf(LuaInfo *li) {
	if (li->name) Memory_Free((void *)li->name);
	if (li->home) Memory_Free((void *)li->home);
	li->hotreload = false; li->version = 0;
}

void runcallback(ELuaEvent type, LuaScript *scr) {
	LuaInfo li = {
		.name = String_AllocCopy(scr->scrname),
		// Заменить первые два NULLа на указатель на домашнюю страницу скрипта, когда она появится
		.home = NULL ? NULL : NULL,
		.hotreload = scr->hotreload,
		.id = scr->id,
		.version = 1
	};

	for (cs_uint32 i = 0; i < MAX_LUA_CALLBACKS; i++)
		if (callbacks[i]) callbacks[i](type, &li);

	if (li.name) Memory_Free((void *)li.name);
	if (li.home) Memory_Free((void *)li.home);
}

LuaItf litf = {
	.addCallback = addcallback,
	.removeCallback = removecallback,

	.lockScriptList = locklist,
	.unlockScriptList = unlocklist,
	.runScriptCommand = runcommand,

	.requestScriptInfo = reqscrinf,
	.discardScriptInfo = disscrinf
};

Plugin_DeclareInterfaces {
	PLUGIN_IFACE_ADD("LuaController_v1", litf),

	PLUGIN_IFACE_END
};
