#include <core.h>
#include <str.h>
#ifdef CSSCRIPTS_PROFILE_MEMORY
#include <log.h>
#endif
#include <platform.h>
#include "luascript.h"
#include "luaclient.h"
#include "luavector.h"
#include "luaangle.h"
#include "luacolor.h"
#include "luablock.h"
#include "luaworld.h"
#include "luacommand.h"
#include "lualog.h"
#include "luasurvival.h"
#include "luaconfig.h"
#include "luacuboid.h"
#include "luagroup.h"
#include "luamodel.h"
#include "luakey.h"
#include "luacontact.h"
#include "luaparticle.h"
#include "luaserver.h"
#include "luaitf.h"

int scr_checktabfield(scr_Context *L, int idx, cs_str fname, int ftype) {
	scr_gettabfield(L, idx, fname);
	if(!scr_istype(L, -1, ftype)) {
		scr_fmterror(L, "Field '%s' must be a %s", fname, scr_typename(L, ftype));
		return false;
	}

	return true;
}

int scr_checktabfieldud(scr_Context *L, int idx, const char *fname, const char *meta) {
	scr_gettabfield(L, idx, fname);
	if(!scr_testmemtype(L, -1, meta)) {
		scr_fmterror(L, "Field '%s' must be a %s", fname, meta);
		return false;
	}

	return true;
}

static const scr_RegFuncs serverlibs[] = {
	{"log", luaopen_log},
	{"key", luaopen_key},
	{"block", luaopen_block},
	{"world", luaopen_world},
	{"client", luaopen_client},
	{"config", luaopen_config},
	{"command", luaopen_command},
	{"vector", luaopen_vector},
	{"angle", luaopen_angle},
	{"color", luaopen_color},
	{"group", luaopen_group},
	{"model", luaopen_model},
	{"particle", luaopen_particle},
	{"contact", luaopen_contact},
	{"server", luaopen_server},
	{"survival", luaopen_survival},

	{NULL, NULL}
};

Script *Script_GetHandle(scr_Context *L) {
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RSCPTR);
	Script *ud = (Script *)scr_getmem(L, -1);
	scr_stackpop(L, 1);
	return ud;
}

cs_bool Script_DoMainFile(Script *script) {
	if(script->unloaded) return false;

	if(luaL_dofile(script->L, script->path)) {
		Script_PrintError(script);
		return false;
	}

	if(Script_GlobalLookup(script, "onStart"))
		return Script_Call(script, 0, 0);

	return true;
}

cs_bool Script_GlobalLookup(Script *script, cs_str key) {
	if(script->unloaded) return false;

	lua_getglobal(script->L, key);
	if(scr_isnull(script->L, -1)) {
		scr_stackpop(script->L, 1);
		return false;
	}

	return true;
}

cs_bool Script_RegistryLookup(Script *script, cs_str regtab, cs_str key) {
	if(script->unloaded) return false;

	scr_gettabfield(script->L, LUA_REGISTRYINDEX, regtab);
	scr_gettabfield(script->L, -1, key);
	if(scr_isnull(script->L, -1)) {
		scr_stackpop(script->L, 1);
		return false;
	}

	return true;
}

cs_bool Script_Call(Script *script, int args, int ret) {
	if(script->unloaded) return false;

	if(scr_protectedcall(script->L, args, ret, 0) != 0) {
		Script_PrintError(script);
		return false;
	}

	return true;
}

static int allowhotreload(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TBOOLEAN);
	Script *script = Script_GetHandle(L);
	script->hotreload = scr_toboolean(L, 1);
	return 0;
}

static int setinfo(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	Script *script = Script_GetHandle(L);
	scr_checktabfield(L, 1, "home", LUA_TSTRING);
	scr_checktabfield(L, 1, "version", LUA_TNUMBER);
	scr_checktabfield(L, 1, "hotreload", LUA_TBOOLEAN);
	if(script->home) Memory_Free((void *)script->home);
	script->hotreload = scr_toboolean(L, -1);
	script->version = (cs_uint32)scr_tointeger(L, -2);
	script->home = String_AllocCopy(scr_tostring(L, -3));
	runcallback(LUAEVENT_UPDATESCRIPT, script);
	script->infoupd = true;
	return 0;
}

static int mstime(scr_Context *L) {
	scr_pushnumber(L, Time_GetMSecD());
	return 1;
}

static int ioensure(scr_Context *L) {
	cs_str path = scr_checkstring(L, 1);
	scr_pushboolean(L, Directory_Ensure(path));
	return 1;
}

static int ioscrname(scr_Context *L) {
	Script *script = Script_GetHandle(L);
	cs_str ext = String_LastChar(script->name, '.');
	scr_pushlstring(L, script->name, ext - script->name);
	return 1;
}

static int iodatafolder(scr_Context *L) {
	int argc = scr_stacktop(L);
	scr_pushstring(L, CSSCRIPTS_PATH_LDATA);
	ioscrname(L);
	if(argc > 0 && scr_isstring(L, 1)) {
		scr_pushstring(L, PATH_DELIM);
		scr_stackpush(L, 1);
		lua_concat(L, 4);
	} else
		lua_concat(L, 2);
	return 1;
}

static cs_str const iodel[] = {
	"input", "output", "stdout",
	"stderr", "stdin", "read",
	"write", NULL
};

static cs_str const osdel[] = {
	"setlocale", "execute",
	"exit", "getenv", NULL
};

static const scr_RegFuncs iofuncs[] = {
	{"ensure", ioensure},
	{"datafolder", iodatafolder},
	{"scrname", ioscrname},

	{NULL, NULL}
};

#ifdef CSSCRIPTS_PROFILE_MEMORY
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
	Script *script = (Script *)ud;

	if(nsize == 0 && ptr)
		script->nfrees++;
	else if(nsize > 0) {
		if(ptr)
			script->nreallocs++;
		else
			script->nallocs++;
	}

	return script->af(script->ad, ptr, osize, nsize);
}
#endif

Script *Script_Open(cs_str name, cs_uint32 id) {
	if(!String_IsSafe(name)) return NULL;
	// TODO: Проверять, успешно и полностью ли собрались пути до файлов

	cs_size offset;
	cs_char path[MAX_PATH_LEN];
	offset = String_Copy(path, MAX_PATH_LEN, CSSCRIPTS_PATH_SCRIPTS);
	String_Append(path, MAX_PATH_LEN, name);
	if(File_Access(path, 04)) {
		offset = String_Copy(path, MAX_PATH_LEN, CSSCRIPTS_PATH_RSCRIPTS);
		String_Append(path, MAX_PATH_LEN, name);
		if(File_Access(path, 04)) return NULL;
	}

	Script *script = Memory_TryAlloc(1, sizeof(Script));

	if(script) {
		script->id = id;
		script->path = String_AllocCopy(path);
		script->name = script->path + offset;
		script->lock = Mutex_Create();
		script->L = scr_contextcreate();
		script->version = 1;
		if(!script->L) {
			Script_Close(script);
			return NULL;
		}

#		ifdef CSSCRIPTS_PROFILE_MEMORY
			script->af = lua_getallocf(script->L, &script->ad);
			lua_setallocf(script->L, l_alloc, script);
#		endif

		scr_pushptr(script->L, script);
		scr_settabfield(script->L, LUA_REGISTRYINDEX, CSSCRIPTS_RSCPTR);

		scr_pushnativefunc(script->L, allowhotreload);
		lua_setglobal(script->L, "allowHotReload");

		scr_pushnativefunc(script->L, setinfo);
		lua_setglobal(script->L, "setInfo");

		scr_pushnativefunc(script->L, mstime);
		lua_setglobal(script->L, "msTime");

		luaL_openlibs(script->L);
		for(const scr_RegFuncs *lib = serverlibs; lib->func; lib++) {
#			if LUA_VERSION_NUM < 502
				scr_pushnativefunc(script->L, lib->func);
				scr_pushstring(script->L, lib->name);
				scr_unprotectedcall(script->L, 1, 1);
				lua_setglobal(script->L, lib->name);
#			else
				luaL_requiref(script->L, lib->name, lib->func, 1);
				scr_stackpop(script->L, 1);
#			endif
		}

		luainit_cuboid(script->L);

		if(Script_GlobalLookup(script, LUA_IOLIBNAME)) {
			for(cs_int32 i = 0; iodel[i]; i++) {
				scr_pushnull(script->L);
				scr_settabfield(script->L, -2, iodel[i]);
			}
			luaL_setfuncs(script->L, iofuncs, 0);
			scr_stackpop(script->L, 1);
		}

		if(Script_GlobalLookup(script, LUA_OSLIBNAME)) {
			for(cs_int32 i = 0; osdel[i]; i++) {
				scr_pushnull(script->L);
				scr_settabfield(script->L, -2, osdel[i]);
			}
			scr_stackpop(script->L, 1);
		}

		lua_getglobal(script->L, "package");
		scr_pushstring(script->L, CSSCRIPTS_PATH);
		scr_settabfield(script->L, -2, "path");
		scr_pushstring(script->L, CSSCRIPTS_CPATH);
		scr_settabfield(script->L, -2, "cpath");
		scr_stackpop(script->L, 1);

		if(!Script_DoMainFile(script)) {
			Script_Close(script);
			return NULL;
		}

		return script;
	}

	return NULL;
}

cs_bool Script_Close(Script *script) {
	if(script->L) scr_contextclose(script->L);
#	ifdef CSSCRIPTS_PROFILE_MEMORY
		Log_Info("%s made %u allocs, %u reallocs, %u frees",
			script->name, script->nallocs,
			script->nreallocs, script->nfrees
		);
#	endif
	Memory_Free((void *)script->path);
	Mutex_Free(script->lock);
	Memory_Free(script);
	return false;
}
