#include <core.h>
#include <log.h>
#include <str.h>
#include <command.h>
#include <plugin.h>
#include <pager.h>

#include "luamain.h"
#include "luaitf.h"
#include "luascript.h"
#include "luaevent.h"

Script *scripts[MAX_SCRIPTS_COUNT] = {NULL};
Mutex *listMutex = NULL;

Plugin_SetVersion(1);
#if PLUGIN_API_NUM > 1
Plugin_SetURL("https://github.com/igor725/cs-lua");
#endif

static cs_uint32 getfreescriptid(void) {
	for (cs_uint32 i = 0; i < MAX_SCRIPTS_COUNT; i++)
		if (scripts[i] == NULL) return i;

	return (cs_uint32)-1;
}

static Script *getscript(cs_str name) {
	for (cs_uint32 i = 0; i < MAX_SCRIPTS_COUNT; i++) {
		Script *script = scripts[i];
		if(script && String_CaselessCompare(script->name, name)) return script;
	}

	return NULL;
}

static Script *LuaLoad(cs_str name) {
	Mutex_Lock(listMutex);
	cs_uint32 id = getfreescriptid();
	if (id == (cs_uint32)-1) {
		Log_Error("Failed to allocate id for script, too many scripts loaded already");
		Mutex_Unlock(listMutex);
		return NULL;
	}

	Script *script = Script_Open(name, id);
	if(script) {
		scripts[id] = script;
		if (!script->infoupd)
			runcallback(LUAEVENT_UPDATESCRIPT, script);
		Mutex_Unlock(listMutex);
		return script;
	}

	Log_Error("Failed to load specified script");
	Mutex_Unlock(listMutex);
	return NULL;
}

cs_bool LuaReload(Script *script) {
	Script_Lock(script);
	if(script->unloaded) {
		Script_Unlock(script);
		return false;
	}

	if(Script_GlobalLookup(script, "preReload")) {
		if(!Script_Call(script, 0, 1)) {
			noreload:
			Script_Unlock(script);
			return false;
		}
		if(!scr_isnull(script->L, -1) && !scr_toboolean(script->L, -1)) {
			scr_stackpop(script->L, 1);
			goto noreload;
		}
	}

	Script_DoMainFile(script);
	Script_Unlock(script);
	return true;
}

cs_bool LuaUnload(Script *script, cs_bool force) {
	if(script->unloaded) return false;

	Script_Lock(script);
	if(Script_GlobalLookup(script, "onStop")) {
		scr_pushboolean(script->L, force);
		if(!Script_Call(script, 1, 1)) {
			goto unlerr;
		} else {
			if(!scr_isnull(script->L, -1) && !scr_toboolean(script->L, -1)) {
				scr_stackpop(script->L, 1);
				goto unlerr;
			}
		}
		goto unlok;
	} else goto unlok;

	unlerr:
	if(!force) {
		Script_Unlock(script);
		return false;
	}

	unlok:
	script->unloaded = true;
	runcallback(LUAEVENT_REMOVESCRIPT, script);
	Script_Unlock(script);
	return true;
}

COMMAND_FUNC(Lua) {
	COMMAND_SETUSAGE("/lua <list/load/unload/reload> [scriptname]");

	cs_char temparg1[64], temparg2[64];
	if(COMMAND_GETARG(temparg1, 64, 0)) {
		if(String_CaselessCompare(temparg1, "list")) {
			cs_uint32 idx = 0, startPage = 1;
			if(COMMAND_GETARG(temparg2, 8, 1))
				startPage = String_ToInt(temparg2);
			Pager pager = Pager_Init(startPage, PAGER_DEFAULT_PAGELEN);
			COMMAND_APPEND("&eList of loaded Lua scripts&f:");

			for (cs_uint32 i = 0; i < MAX_SCRIPTS_COUNT; i++) {
				Script *script = scripts[i];
				if (!script) continue;
				idx += 1;
				Pager_Step(pager);

				Script_Lock(script);
				int usage = scr_contextusage(script->L);
				COMMAND_APPENDF(temparg1, 64, "\r\n  %d. &9%.32s&f, &a%dKb&f used", idx, script->name, usage);
				Script_Unlock(script);
			}

			if(Pager_IsDirty(pager))
				COMMAND_APPENDF(temparg1, 64, "\r\nPage %d/%d shown",
					Pager_CurrentPage(pager), Pager_CountPages(pager)
				);

			return true;
		} else if(String_CaselessCompare(temparg1, "version")) {
			COMMAND_PRINTF("Lua plugin v%03d (%s) on %s", Plugin_Version, GIT_COMMIT_TAG, CSSCRIPTS_LIBVERSION);
		} else {
			if(!COMMAND_GETARG(temparg2, 64, 1))
				COMMAND_PRINTUSAGE;

			if(!String_FindSubstr(temparg2, ".lua"))
				String_Append(temparg2, 64, ".lua");

			Script *script = getscript(temparg2);

			if(String_CaselessCompare(temparg1, "load")) {
				if(script)
					COMMAND_PRINT("&cThis script is already loaded");
				if(LuaLoad(temparg2))
					COMMAND_PRINTF("&aScript \"%s\" loaded successfully", temparg2);
				else
					COMMAND_PRINT("&cFailed to load specified script");
			} else {
				if(!script)
					COMMAND_PRINTF("&cScript \"%s\" not found", temparg2);

				if(String_CaselessCompare(temparg1, "unload")) {
					cs_bool force = false;
					if(COMMAND_GETARG(temparg1, 64, 2))
						force = String_CaselessCompare(temparg1, "force");

					if(LuaUnload(script, force))
						COMMAND_PRINT("&aScript unloaded successfully");
					else
						COMMAND_PRINT("&cThis script cannot be unloaded right now");
				} else if(String_CaselessCompare(temparg1, "reload")) {
					if(!script->hotreload)
						COMMAND_PRINT("&cThis script does not support hot reloading");

					if(LuaReload(script))
						COMMAND_PRINT("&aScript reloaded successfully");
					else
						COMMAND_PRINT("&cThis script cannot be reloaded right now");
				}
			}
		}
	}

	COMMAND_PRINTUSAGE;
}

#ifdef CSSCRIPTS_USE_SURVIVAL
#include "cs-survival/src/survitf.h"

void *SurvInterface = NULL;

void Plugin_RecvInterface(cs_str name, void *ptr, cs_size size) {
	if(String_Compare(name, SURV_ITF_NAME)) {
		if(size == sizeof(SurvItf) || (SurvInterface != NULL && size == 0)) {
			if(SurvInterface) Memory_Free(SurvInterface);
			SurvInterface = ptr;
		} else
			Log_Error("Script failed to bind SurvItf: Structure size mismatch");
	}
}
#endif

static cs_bool checkscrname(cs_str name) {
	if(*name == '.') {
		Log_Warn("File \"%s\" was ignored: File name starts with a dot", name);
		return false;
	}

	return true;
}

cs_bool Plugin_Load(void) {
	DirIter sIter = {0};
	Directory_Ensure(CSSCRIPTS_PATH_LDATA); // Папка с данными для каждого скрипта
	Directory_Ensure(CSSCRIPTS_PATH_LROOT); // Папка для библиотек, подключаемых скриптами
	Directory_Ensure(CSSCRIPTS_PATH_CROOT); // Папка для C модулей, подключаемых скриптами
	Directory_Ensure(CSSCRIPTS_PATH_SCRIPTS); // Сами скрипты, загружаются автоматически
	listMutex = Mutex_Create();

	if(Iter_Init(&sIter, CSSCRIPTS_PATH_SCRIPTS, "lua")) { // Проходимся по директории зависящей от версии Lua
		do {
			if(sIter.isDir || !sIter.cfile) continue;
			if(!checkscrname(sIter.cfile)) continue;
			LuaLoad(sIter.cfile);
		} while(Iter_Next(&sIter));
	}
	Iter_Close(&sIter);

	if(Iter_Init(&sIter, CSSCRIPTS_PATH_RSCRIPTS, "lua")) { // Проходимся по общей директории
		do {
			if(sIter.isDir || !sIter.cfile) continue;
			if(getscript(sIter.cfile)) {
				Log_Warn("File %s%s was ignored: script with the same name has already been loaded",
					CSSCRIPTS_PATH_RSCRIPTS, sIter.cfile
				);
				continue;
			}
			if(!checkscrname(sIter.cfile)) continue;
			LuaLoad(sIter.cfile);
		} while(Iter_Next(&sIter));
	}
	Iter_Close(&sIter);

	return COMMAND_ADD(Lua, CMDF_OP, "Lua scripts management")
	&& LuaEvent_Register();
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	COMMAND_REMOVE(Lua);
	LuaEvent_Unregister();
	if (listMutex) Mutex_Free(listMutex);

#	ifdef CSSCRIPTS_USE_SURVIVAL
		if(SurvInterface)
			Memory_Free(SurvInterface);
#	endif

	for (cs_uint32 i = 0; i < MAX_SCRIPTS_COUNT; i++) {
		Script *script = scripts[i];
		if (!script) continue;
		Script_Lock(script);
		scripts[i] = NULL;
		LuaUnload(script, true);
		Script_Unlock(script);
		Script_Close(script);
	}

	return true;
}
