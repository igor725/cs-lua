#ifndef CSLUAMAIN_H
#define CSLUAMAIN_H
#include <core.h>
#include <platform.h>
#include "luascript.h"

#define MAX_SCRIPTS_COUNT 128u
extern LuaScript *scripts[MAX_SCRIPTS_COUNT];
extern Mutex *listMutex;

#define LuaScriptList_Iter(body) \
Mutex_Lock(listMutex); \
for (cs_uint32 _si = 0; _si < MAX_SCRIPTS_COUNT; _si++) { \
	LuaScript *script = scripts[_si]; \
	if (!script) { continue; } \
	body \
} \
Mutex_Unlock(listMutex);

#define LuaScriptList_RemoveCurrent() \
scripts[_si] = NULL

cs_bool LuaReload(LuaScript *script);
cs_bool LuaUnload(LuaScript *script, cs_bool force);
#endif
