#include <core.h>
#include <str.h>
#ifdef CSLUA_PROFILE_MEMORY
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

// Слой совместимости для чистой версии Lua 5.1
#ifdef CSLUA_NONJIT_51
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
	for(; l->name != NULL; l++) {
		int i;
		for(i = 0; i < nup; i++)
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, l->func, nup);
		lua_setfield(L, -(nup + 2), l->name);
	}
	lua_pop(L, nup);
}

void luaL_setmetatable(lua_State *L, const char *tname) {
	luaL_getmetatable(L, tname);
	lua_setmetatable(L, -2);
}

void *luaL_testudata (lua_State *L, int ud, const char *tname) {
	void *p = lua_touserdata(L, ud);
	if(p != NULL) {
		if(lua_getmetatable(L, ud)) {
			luaL_getmetatable(L, tname);
			if(!lua_rawequal(L, -1, -2))
				p = NULL;
			lua_pop(L, 2);
			return p;
		}
	}
	return NULL;
}
#endif

int lua_checktabfield(lua_State *L, int idx, cs_str fname, int ftype) {
	lua_getfield(L, idx, fname);
	if(lua_type(L, -1) != ftype) {
		luaL_error(L, "Field '%s' must be a %s", fname, lua_typename(L, ftype));
		return false;
	}

	return true;
}

int lua_checktabfieldud(lua_State *L, int idx, const char *fname, const char *meta) {
	lua_getfield(L, idx, fname);
	if(!luaL_testudata(L, -1, meta)) {
		luaL_error(L, "Field '%s' must be a %s", fname, meta);
		return false;
	}

	return true;
}

void lua_indexedmeta(lua_State *L, const char *meta, const luaL_Reg *meths) {
	if(!luaL_newmetatable(L, meta)) luaL_error(L, "Failed to create metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
#	if LUA_VERSION_NUM < 503
		lua_pushstring(L, meta);
		lua_setfield(L, -2, "__name");
#	endif
	lua_pushstring(L, "Huh? Tf you doing here?");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, meths, 0);
	lua_pop(L, 1);
}

static const luaL_Reg lualibs[] = {
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
	{"survival", luaopen_survival},

	{NULL,NULL}
};

LuaScript *lua_getscript(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, CSLUA_RSCPTR);
	LuaScript *ud = (LuaScript *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return ud;
}

cs_bool LuaScript_DoMainFile(LuaScript *script) {
	if(script->unloaded) return false;

	if(luaL_dofile(script->L, script->scrpath)) {
		LuaScript_PrintError(script);
		return false;
	}

	if(LuaScript_GlobalLookup(script, "onStart"))
		return LuaScript_Call(script, 0, 0);

	return true;
}

cs_bool LuaScript_GlobalLookup(LuaScript *script, cs_str key) {
	if(script->unloaded) return false;

	lua_getglobal(script->L, key);
	if(lua_isnil(script->L, -1)) {
		lua_pop(script->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaScript_RegistryLookup(LuaScript *script, cs_str regtab, cs_str key) {
	if(script->unloaded) return false;

	lua_getfield(script->L, LUA_REGISTRYINDEX, regtab);
	lua_getfield(script->L, -1, key);
	if(lua_isnil(script->L, -1)) {
		lua_pop(script->L, 1);
		return false;
	}

	return true;
}

cs_bool LuaScript_Call(LuaScript *script, int args, int ret) {
	if(script->unloaded) return false;

	if(lua_pcall(script->L, args, ret, 0) != 0) {
		LuaScript_PrintError(script);
		return false;
	}

	return true;
}

static int allowhotreload(lua_State *L) {
	luaL_checktype(L, 1, LUA_TBOOLEAN);
	LuaScript *script = lua_getscript(L);
	script->hotreload = (cs_bool)lua_toboolean(L, 1);
	return 0;
}

static int mstime(lua_State *L) {
	lua_pushnumber(L, Time_GetMSecD());
	return 1;
}

static int ioensure(lua_State *L) {
	cs_str path = luaL_checkstring(L, 1);
	lua_pushboolean(L, Directory_Ensure(path));
	return 1;
}

static int ioscrname(lua_State *L) {
	LuaScript *script = lua_getscript(L);
	cs_str ext = String_LastChar(script->scrname, '.');
	lua_pushlstring(L, script->scrname, ext - script->scrname);
	return 1;
}

static int iodatafolder(lua_State *L) {
	lua_pushstring(L, CSLUA_PATH_LDATA);
	ioscrname(L);
	lua_concat(L, 2);
	return 1;
}

static cs_str const iodel[] = {
	"input", "output", "stdout",
	"stderr", "stdin", "read",
	"write", NULL
};

static cs_str const osdel[] = {
	"setlocale", "execute", "exit",
	"getenv", NULL
};

static const luaL_Reg iofuncs[] = {
	{"ensure", ioensure},
	{"datafolder", iodatafolder},
	{"scrname", ioscrname},

	{NULL, NULL}
};

#ifdef CSLUA_PROFILE_MEMORY
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
	LuaScript *script = (LuaScript *)ud;

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

LuaScript *LuaScript_Open(cs_str name) {
	if(String_FindSubstr(name, ".."))
		return NULL;

	cs_size offset;
	cs_char path[MAX_PATH];
	offset = String_Copy(path, MAX_PATH, CSLUA_PATH_SCRIPTS);
	String_Append(path, MAX_PATH, name);
	if(File_Access(path, 04)) {
		offset = String_Copy(path, MAX_PATH, CSLUA_PATH_RSCRIPTS);
		String_Append(path, MAX_PATH, name);
		if(File_Access(path, 04)) return NULL;
	}

	LuaScript *script = Memory_TryAlloc(1, sizeof(LuaScript));

	if(script) {
		script->scrpath = String_AllocCopy(path);
		script->scrname = script->scrpath + offset;
		script->lock = Mutex_Create();
		script->L = luaL_newstate();
		if(!script->L) {
			LuaScript_Close(script);
			return NULL;
		}

#		ifdef CSLUA_PROFILE_MEMORY
			script->af = lua_getallocf(script->L, &script->ad);
			lua_setallocf(script->L, l_alloc, script);
#		endif

		lua_pushlightuserdata(script->L, script);
		lua_setfield(script->L, LUA_REGISTRYINDEX, CSLUA_RSCPTR);

		lua_pushcfunction(script->L, allowhotreload);
		lua_setglobal(script->L, "allowHotReload");

		lua_pushcfunction(script->L, mstime);
		lua_setglobal(script->L, "msTime");

		luaL_openlibs(script->L);
		for(const luaL_Reg *lib = lualibs; lib->func; lib++) {
#			if LUA_VERSION_NUM < 502
				lua_pushcfunction(script->L, lib->func);
				lua_pushstring(script->L, lib->name);
				lua_call(script->L, 1, 1);
				lua_setglobal(script->L, lib->name);
#			else
				luaL_requiref(script->L, lib->name, lib->func, 1);
				lua_pop(script->L, 1);
#			endif
		}

		luainit_cuboid(script->L);

		if(LuaScript_GlobalLookup(script, LUA_IOLIBNAME)) {
			for(cs_int32 i = 0; iodel[i]; i++) {
				lua_pushnil(script->L);
				lua_setfield(script->L, -2, iodel[i]);
			}
			luaL_setfuncs(script->L, iofuncs, 0);
			lua_pop(script->L, 1);
		}

		if(LuaScript_GlobalLookup(script, LUA_OSLIBNAME)) {
			for(cs_int32 i = 0; osdel[i]; i++) {
				lua_pushnil(script->L);
				lua_setfield(script->L, -2, osdel[i]);
			}
			lua_pop(script->L, 1);
		}

		lua_getglobal(script->L, "package");
		lua_pushstring(script->L, CSLUA_PATH);
		lua_setfield(script->L, -2, "path");
		lua_pushstring(script->L, CSLUA_CPATH);
		lua_setfield(script->L, -2, "cpath");
		lua_pop(script->L, 1);

		if(!LuaScript_DoMainFile(script)) {
			LuaScript_Close(script);
			return NULL;
		}

		return script;
	}

	return NULL;
}

cs_bool LuaScript_Close(LuaScript *script) {
	if(script->L) lua_close(script->L);
#	ifdef CSLUA_PROFILE_MEMORY
		Log_Info("%s made %u allocs, %u reallocs, %u frees",
			script->scrname, script->nallocs,
			script->nreallocs, script->nfrees
		);
#	endif
	Memory_Free((void *)script->scrpath);
	Mutex_Free(script->lock);
	Memory_Free(script);
	return false;
}
