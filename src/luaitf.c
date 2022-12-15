#include <core.h>
#include <plugin.h>
#include <str.h>

#include "luamain.h"
#include "luascript.h"
#include "luaitf.h"

LuaEventFunc callbacks[CSLUA_MAX_CALLBACKS] = {NULL};

static cs_bool addcallback(LuaEventFunc func) {
	for (cs_uint32 i = 0; i < CSLUA_MAX_CALLBACKS; i++) {
		if (!callbacks[i]) {
			callbacks[i] = func;
			return true;
		}
	}

	return false;
}

static cs_bool removecallback(LuaEventFunc func) {
	for (cs_uint32 i = 0; i < CSLUA_MAX_CALLBACKS; i++) {
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
	LuaScript *script;
	if (idx >= MAX_SCRIPTS_COUNT || (script = scripts[idx]) == NULL)
		return false;

	switch (cmd) {
		case LUACOMMAND_RELOAD:
			return LuaReload(script);
		case LUACOMMAND_UNLOAD:
		case LUACOMMAND_FORCEUNLOAD:
			return LuaUnload(script, cmd == LUACOMMAND_FORCEUNLOAD);
	}

	return false;
}

static cs_uint32 reqscrinf(LuaInfo *li, cs_uint32 id) {
	LuaScript *ptr = NULL;
	while (!ptr && id < MAX_SCRIPTS_COUNT)
		ptr = scripts[id++];
	if (id >= MAX_SCRIPTS_COUNT) return 0;
	li->name = String_AllocCopy(ptr->name);
	li->home = ptr->home ? String_AllocCopy(ptr->home) : NULL;
	li->hotreload = ptr->hotreload;
	li->version = ptr->version;
	li->id = ptr->id;

	for (; id < MAX_SCRIPTS_COUNT; id++)
		if (scripts[id]) return id;

	return id;
}

static void disscrinf(LuaInfo *li) {
	if (li->name) Memory_Free((void *)li->name);
	if (li->home) Memory_Free((void *)li->home);
	li->hotreload = false; li->version = 0;
}

void runcallback(ELuaEvent type, LuaScript *scr) {
	LuaInfo li = {
		.id = scr->id
	};

	if (type == LUAEVENT_UPDATESCRIPT) {
		li.name = String_AllocCopy(scr->name);
		li.home = scr->home ? String_AllocCopy(scr->home) : NULL;
		li.hotreload = scr->hotreload;
		li.version = scr->version;
	}

	for (cs_uint32 i = 0; i < CSLUA_MAX_CALLBACKS; i++)
		if (callbacks[i]) callbacks[i](type, type == LUAEVENT_UPDATESCRIPT ? (void *)&li : (void *)&li.id);

	if (type == LUAEVENT_UPDATESCRIPT) disscrinf(&li);
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
	PLUGIN_IFACE_ADD(CSLUA_ITF_NAME, litf),

	PLUGIN_IFACE_END
};
