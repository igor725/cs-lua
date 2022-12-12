#ifndef LUAITF_H
#define LUAITF_H
#include <core.h>

#include "luascript.h"
#define CSLUA_MAX_CALLBACKS 64u
#define CSLUA_ITF_NAME "LuaController_v1"

typedef struct _LuaInfo {
	cs_uint32 id, version;
	cs_str name, home;
	cs_bool hotreload;
} LuaInfo;

typedef enum _ELuaEvent {
	LUAEVENT_ADDSCRIPT,
	LUAEVENT_REMOVESCRIPT,
	LUAEVENT_UPDATEINFO
} ELuaEvent;

typedef enum _ELuaCommand {
	LUACOMMAND_RELOAD,
	LUACOMMAND_UNLOAD
} ELuaCommand;

typedef void(*LuaEventFunc)(ELuaEvent type, const void *li);

typedef struct _LuaItf {
	cs_bool(*addCallback)(LuaEventFunc func);
	cs_bool(*removeCallback)(LuaEventFunc func);

	void(*lockScriptList)(void);
	void(*unlockScriptList)(void);
	cs_bool(*runScriptCommand)(ELuaCommand cmd, cs_uint32 id);

	cs_uint32(*requestScriptInfo)(LuaInfo *li, cs_uint32 id);
	void(*discardScriptInfo)(LuaInfo *li);
} LuaItf;

void runcallback(ELuaEvent type, LuaScript *scr);
#endif
