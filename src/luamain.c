#include <core.h>
#include <list.h>
#include <log.h>
#include <str.h>
#include <command.h>
#include <plugin.h>
#include <pager.h>
#include "luamain.h"
#include "luascript.h"
#include "luaevent.h"

#define DISABLED_DIR "scripts" PATH_DELIM "disabled"
#define CLIBS_DIR "lua" PATH_DELIM "clibs"

AListField *headScript = NULL;

Plugin_SetVersion(1);

static LuaScript *getscript(cs_str name) {
	AListField *tmp;
	List_Iter(tmp, headScript) {
		LuaScript *script = getscriptptr(tmp);
		if(String_Compare(script->scrname, name)) return script;
	}

	return NULL;
}

static LuaScript *LuaLoad(cs_str name) {
	LuaScript *script = LuaScript_Open(name);
	if(script) AList_AddField(&headScript, script);
	return script;
}

static cs_bool LuaReload(LuaScript *script) {
	LuaScript_Lock(script);
	if(script->unloaded) {
		LuaScript_Unlock(script);
		return false;
	}

	if(LuaScript_GlobalLookup(script, "preReload")) {
		if(!LuaScript_Call(script, 0, 1)) {
			noreload:
			LuaScript_Unlock(script);
			return false;
		}
		if(!lua_isnil(script->L, -1) && !lua_toboolean(script->L, -1)) {
			lua_pop(script->L, 1);
			goto noreload;
		}
	}

	LuaScript_DoMainFile(script);
	LuaScript_Unlock(script);
	return true;
}

static cs_bool LuaUnload(LuaScript *script, cs_bool force) {
	if(script->unloaded) return false;

	LuaScript_Lock(script);
	if(LuaScript_GlobalLookup(script, "onStop")) {
		lua_pushboolean(script->L, force);
		if(!LuaScript_Call(script, 1, 1)) {
			goto unlerr;
		} else {
			if(!lua_isnil(script->L, -1) && !lua_toboolean(script->L, -1)) {
				lua_pop(script->L, 1);
				goto unlerr;
			}
		}
		goto unlok;
	}

	unlerr:
	if(!force) {
		LuaScript_Unlock(script);
		return false;
	}

	unlok:
	script->unloaded = true;
	LuaScript_Unlock(script);
	return true;
}

COMMAND_FUNC(Lua) {
	COMMAND_SETUSAGE("/lua <list/load/unload/reload/disable/enable> [scriptname]");

	cs_char temparg1[64], temparg2[64];
	if(COMMAND_GETARG(temparg1, 64, 0)) {
		if(String_CaselessCompare(temparg1, "list")) {
			cs_int32 idx = 0, startPage = 1;
			if(COMMAND_GETARG(temparg2, 8, 1))
				startPage = String_ToInt(temparg2);
			Pager pager = Pager_Init(startPage, PAGER_DEFAULT_PAGELEN);
			COMMAND_APPEND("&eList of loaded Lua scripts&f:");

			AListField *tmp;
			List_Iter(tmp, headScript) {
				idx += 1;
				Pager_Step(pager);

				LuaScript *script = getscriptptr(tmp);
				LuaScript_Lock(script);
				int usage = lua_gc(script->L, LUA_GCCOUNT, 0);
				COMMAND_APPENDF(temparg1, 64, "\r\n  %d. &9%s&f, &a%dKb&f used", idx, script->scrname, usage);
				LuaScript_Unlock(script);
			}

			if(Pager_IsDirty(pager))
				COMMAND_APPENDF(temparg1, 64, "\r\nPage %d/%d shown",
					Pager_CurrentPage(pager), Pager_CountPages(pager)
				);

			return true;
		} else if(String_CaselessCompare(temparg1, "version")) {
			COMMAND_PRINTF("Lua plugin v%03d (%s) on %s", Plugin_Version, GIT_COMMIT_TAG, CSLUA_LIBVERSION);
		} else {
			if(!COMMAND_GETARG(temparg2, 64, 1))
				COMMAND_PRINTUSAGE;

			if(!String_FindSubstr(temparg2, ".lua"))
				String_Append(temparg2, 64, ".lua");

			cs_char oldname[MAX_PATH], newname[MAX_PATH];
			LuaScript *script = getscript(temparg2);

			if(String_CaselessCompare(temparg1, "load")) {
				if(script)
					COMMAND_PRINT("&cThis script is already loaded");
				if(LuaLoad(temparg2))
					COMMAND_PRINTF("&aScript \"%s\" loaded successfully", temparg2);
				else
					COMMAND_PRINT("&cFailed to load specified script");
			} else if(String_CaselessCompare(temparg1, "enable")) {
				if(script)
					COMMAND_PRINT("&cThis script is already enabled");
				if(String_FormatBuf(newname, MAX_PATH, "scripts" PATH_DELIM "%s", temparg2) &&
				String_FormatBuf(oldname, MAX_PATH, DISABLED_DIR PATH_DELIM "%s", temparg2)) {
					if(File_Rename(oldname, newname)) {
						if(LuaLoad(temparg2))
							COMMAND_PRINT("&aScript enabled successfully");
						else
							COMMAND_PRINT("&eScript enabled but not loaded");
					} else
						COMMAND_PRINT("&cFailed to enable specified script");
				} else
					COMMAND_PRINT("&cUnexpected error");
			} else if(String_CaselessCompare(temparg1, "disable")) {
				if(!script)
					COMMAND_PRINT("&cThis script is not loaded");

				if(String_FormatBuf(oldname, MAX_PATH, "scripts" PATH_DELIM "%s", temparg2) &&
				String_FormatBuf(newname, MAX_PATH, DISABLED_DIR PATH_DELIM "%s", temparg2)) {
					if(File_Rename(oldname, newname)) {
						if(LuaUnload(script, false))
							COMMAND_PRINT("&aScript disabled successfully");
						else
							COMMAND_PRINT("&eScript will not be loaded at the next launch");
					} else
						COMMAND_PRINT("&cFailed to disabled specified script");
				} else
					COMMAND_PRINT("&cUnexpected error");
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

#ifdef CSLUA_USE_SURVIVAL
#include "cs-survival/src/survitf.h"

void *SurvInterface = NULL;

void Plugin_RecvInterface(cs_str name, void *ptr, cs_size size) {
	if(String_Compare(name, SURV_ITF_NAME)) {
		if(size == sizeof(SurvItf) || (SurvInterface != NULL && size == 0))
			SurvInterface = ptr;
		else
			Log_Error("LuaScript failed to bind SurvItf: Structure size mismatch");
	}
}
#endif

cs_bool Plugin_Load(void) {
	DirIter sIter;
	Directory_Ensure("lua"); // Папка для библиотек, подключаемых скриптами
	Directory_Ensure(CLIBS_DIR); // Папка для библиотек, подключаемых скриптами
	Directory_Ensure("scripts"); // Сами скрипты, загружаются автоматически
	Directory_Ensure("luadata"); // Папка с данными для каждого скрипта
	Directory_Ensure(DISABLED_DIR); // Сюда будут переноситься выключенные скрипты

	if(Iter_Init(&sIter, "scripts", "lua")) {
		do {
			if(sIter.isDir || !sIter.cfile) continue;
			cs_str ext = String_LastChar(sIter.cfile, '.');
			if((ext - sIter.cfile) < 1) {
				Log_Warn("File \"%s\" ignored", sIter.cfile);
				continue;
			}
			LuaScript *script = LuaScript_Open(sIter.cfile);
			if(!script) {
				Log_Error("Failed to load script \"%s\"", sIter.cfile);
				continue;
			}
			AList_AddField(&headScript, script);
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

	while(headScript) {
		LuaScript *script = (LuaScript *)AList_GetValue(headScript).ptr;
		LuaScript_Lock(script);
		AList_Remove(&headScript, headScript);
		LuaUnload(script, true);
		LuaScript_Unlock(script);
		LuaScript_Close(script);
	}

	return true;
}
