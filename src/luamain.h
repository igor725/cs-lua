#ifndef CSLUAMAIN_H
#define CSLUAMAIN_H
#include <core.h>
#include <platform.h>
#include "luascript.h"

#define MAX_SCRIPTS_COUNT 128u
extern Script *scripts[MAX_SCRIPTS_COUNT];
extern Mutex *listMutex;

#define ScriptList_Iter(body) \
Mutex_Lock(listMutex); \
for (cs_uint32 _si = 0; _si < MAX_SCRIPTS_COUNT; _si++) { \
	Script *script = scripts[_si]; \
	if (!script) { continue; } \
	body \
} \
Mutex_Unlock(listMutex);

#define ScriptList_RemoveCurrent() \
scripts[_si] = NULL

cs_bool LuaReload(Script *script);
cs_bool LuaUnload(Script *script, cs_bool force);
#endif
