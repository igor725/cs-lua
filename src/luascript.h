#ifndef CSLUAPLUGIN_H
#define CSLUAPLUGIN_H
#include <core.h>
#include <platform.h>
#include <log.h>
#include "scripting.h"

typedef struct _Script {
	cs_uint32 id, version;
	cs_bool hotreload;
	cs_bool unloaded;
	cs_bool infoupd;
	cs_str name, path, home;
	scr_Context *L;
	Mutex *lock;

#	ifdef CSSCRIPTS_PROFILE_MEMORY
		lua_Alloc af;
		void *ad;
		cs_uint32 nfrees;
		cs_uint32 nallocs;
		cs_uint32 nreallocs;
#	endif
} Script;

// Регистровые таблицы для хранения разных приколов
#define CSSCRIPTS_RSCPTR   "csscptr"
#define CSSCRIPTS_RWORLDS  "csworlds"
#define CSSCRIPTS_RCLIENTS "csclients"
#define CSSCRIPTS_RCMDS    "cscmds"
#define CSSCRIPTS_RCUBOIDS "cscuboids"
#define CSSCRIPTS_RCONTACT "cscontact"

// Метатаблицы разных приколов
#define CSSCRIPTS_MVECTOR   "Vector"
#define CSSCRIPTS_MANGLE    "Angle"
#define CSSCRIPTS_MCOLOR    "Color"
#define CSSCRIPTS_MBLOCK    "Block"
#define CSSCRIPTS_MBULK     "Bulk"
#define CSSCRIPTS_MCONFIG   "Config"
#define CSSCRIPTS_MCLIENT   "Client"
#define CSSCRIPTS_MWORLD    "World"
#define CSSCRIPTS_MCUBOID   "Cuboid"
#define CSSCRIPTS_MMODEL    "Model"
#define CSSCRIPTS_MCONTACT  "Contact"
#define CSSCRIPTS_MPARTICLE "Particle"

#define Script_Lock(p) Mutex_Lock((p)->lock)
#define Script_Unlock(p) Mutex_Unlock((p)->lock)
#define Script_PrintError(p) Log_Error("Script \"%s\" got an error: %s", (p)->name, scr_tostring((p)->L, -1))

Script *Script_GetHandle(scr_Context *L);
Script *Script_Open(cs_str name, cs_uint32 id);
cs_bool Script_DoMainFile(Script *script);
cs_bool Script_GlobalLookup(Script *plugin, cs_str key);
cs_bool Script_RegistryLookup(Script *plugin, cs_str regtab, cs_str key);
cs_bool Script_Call(Script *plugin, int args, int ret);
cs_bool Script_Close(Script *plugin);
#endif
