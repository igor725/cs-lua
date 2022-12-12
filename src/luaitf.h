#ifndef LUAITF_H
#define LUAITF_H
#include <core.h>

#define MAX_LUA_CALLBACKS 64u

typedef struct _LuaInfo {
	cs_uint32 id, version;
	cs_str name, home;
	cs_bool hotreload;
} LuaInfo;

typedef enum _ELuaEvent {
	LUAEVENT_ADDSCRIPT,
	LUAEVENT_REMOVESCRIPT
} ELuaEvent;

typedef enum _ELuaCommand {
	LUACOMMAND_RELOAD,
	LUACOMMAND_UNLOAD
} ELuaCommand;

typedef void(*LuaEventFunc)(ELuaEvent type, LuaInfo *li);

typedef struct _LuaItf {
	cs_bool(*addCallback)(LuaEventFunc func);
	cs_bool(*removeCallback)(LuaEventFunc func);

	void(*lockScriptList)(void);
	void(*unlockScriptList)(void);
	cs_bool(*runScriptCommand)(ELuaCommand cmd, cs_uint32 idx);

	cs_uint32(*requestScriptInfo)(LuaInfo *li, cs_uint32 id);
	void(*discardScriptInfo)(LuaInfo *li);
} LuaItf;

void runcallback(ELuaEvent type, LuaScript *li);
#endif
