#ifndef CSLUAPLUGIN_H
#define CSLUAPLUGIN_H
#ifdef CORE_USE_WINDOWS
#	define LUA_BUILD_AS_DLL
#	define LUA_LIB
#endif
#include <core.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <platform.h>
#include <log.h>

typedef struct LuaScript {
	cs_bool hotreload;
	cs_bool unloaded;
	cs_str scrname;
	lua_State *L;
	Mutex *lock;

#	ifdef CSLUA_PROFILE_MEMORY
		lua_Alloc af;
		void *ad;
		cs_uint32 nfrees;
		cs_uint32 nallocs;
		cs_uint32 nreallocs;
#	endif
} LuaScript;

// Регистровые таблицы для хранения разных приколов
#define CSLUA_RSCPTR   "csscptr"
#define CSLUA_RWORLDS  "csworlds"
#define CSLUA_RCLIENTS "csclients"
#define CSLUA_RCMDS    "cscmds"
#define CSLUA_RCUBOIDS "cscuboids"
#define CSLUA_RCONTACT "cscontact"

// Метатаблицы разных приколов
#define CSLUA_MVECTOR  "Vector"
#define CSLUA_MANGLE   "Angle"
#define CSLUA_MCOLOR   "Color"
#define CSLUA_MBLOCK   "Block"
#define CSLUA_MBULK    "Bulk"
#define CSLUA_MCONFIG  "Config"
#define CSLUA_MCLIENT  "Client"
#define CSLUA_MWORLD   "World"
#define CSLUA_MCUBOID  "Cuboid"
#define CSLUA_MMODEL   "Model"
#define CSLUA_MCONTACT "Contact"

// Обеспечиваем совместимость с большинством версий Lua
#if LUA_VERSION_NUM < 501
// Кто-то вообще будет пытаться собрать плагин с этими версиями?
#	error "This version of Lua is not supported"
#elif LUA_VERSION_NUM > 501 // Что-то старше 5.1
#	define lua_objlen(L, idx) lua_rawlen(L, idx)
#	if LUA_VERSION_NUM == 502 || defined(LUA_COMPAT_BITLIB)
#		define luaopen_bit luaopen_bit32
#		define CSLUA_HAS_BIT
#	endif
#elif !defined(LUA_JITLIBNAME) // Чистый Lua 5.1
#	define CSLUA_NONJIT_51
//	Немного приколов из Lua 5.2
#	define luaL_newlibtable(L, l) \
	lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)
#	define luaL_newlib(L, l)	(luaL_newlibtable(L, l), luaL_setfuncs(L, l, 0))

	void  luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
	void *luaL_testudata(lua_State *L, int ud, const char *tname);
	void  luaL_setmetatable(lua_State *L, const char *tname);
#else // Судя по всему, мы компилимся под JITом
#	include "luajit.h"
#	define CSLUA_HAS_JIT
#	define CSLUA_HAS_BIT
#	define CSLUA_LIBVERSION LUAJIT_VERSION
#endif

#ifndef CSLUA_LIBVERSION
#	define CSLUA_LIBVERSION LUA_VERSION
#endif

#define lua_addnumconst(L, n) (lua_pushnumber(L, n), lua_setglobal(L, #n))
#define lua_addintconst(L, n) (lua_pushinteger(L, n), lua_setglobal(L, #n))
#define LuaScript_Lock(p) Mutex_Lock((p)->lock)
#define LuaScript_Unlock(p) Mutex_Unlock((p)->lock)
#define LuaScript_PrintError(p) Log_Error("Script \"%s\" got an error: %s", (p)->scrname, lua_tostring((p)->L, -1))

int lua_checktabfield(lua_State *L, int idx, const char *fname, int ftype);
int lua_checktabfieldud(lua_State *L, int idx, const char *fname, const char *meta);
int lua_indexedmeta(lua_State *L, const char *meta, const luaL_Reg *meths);

LuaScript *lua_getscript(lua_State *L);
LuaScript *LuaScript_Open(cs_str name);
cs_bool LuaScript_DoMainFile(LuaScript *script);
cs_bool LuaScript_GlobalLookup(LuaScript *plugin, cs_str key);
cs_bool LuaScript_RegistryLookup(LuaScript *plugin, cs_str regtab, cs_str key);
cs_bool LuaScript_Call(LuaScript *plugin, int args, int ret);
cs_bool LuaScript_Close(LuaScript *plugin);
#endif
