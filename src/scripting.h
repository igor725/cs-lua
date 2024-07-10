#ifndef SCRIPTING_H
#define SCRIPTING_H

#ifdef CORE_USE_WINDOWS
#	define LUA_BUILD_AS_DLL
#	define LUA_LIB
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define scr_libfunc(N) luaopen_##N

#define scr_toboolean(C, I) ((cs_bool)lua_toboolean(C, I))

#define scr_istype(C, I, T) (lua_type(L, I) == T)

// Обеспечиваем совместимость с большинством версий Lua
#if LUA_VERSION_NUM < 503 // Компилимся под чем-ито ниже Lua 5.3
#	define lua_absindex(L, idx) ((idx) > 0 || ((idx) <= LUA_REGISTRYINDEX) ? (idx) : lua_gettop(L) + (idx) + 1)
#endif

#if LUA_VERSION_NUM < 501
// Кто-то вообще будет пытаться собрать плагин с этими версиями?
#	error "This version of Lua is not supported"
#elif LUA_VERSION_NUM > 501 // Что-то старше 5.1
#	define lua_objlen(L, idx) lua_rawlen(L, idx)
#	if LUA_VERSION_NUM == 502 || defined(LUA_COMPAT_BITLIB)
#		define luaopen_bit luaopen_bit32
#		define CSSCRIPTS_HAS_BIT
#	endif
#elif !defined(LUA_JITLIBNAME) // Чистый Lua 5.1
#	define CSSCRIPTS_NONJIT_51
//		Немного приколов из Lua 5.2
#	define luaL_newlibtable(L, l) \
	lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)
#	define luaL_newlib(L, l)	(luaL_newlibtable(L, l), luaL_setfuncs(L, l, 0))

	void  luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
	void *luaL_testudata(lua_State *L, int ud, const char *tname);
	void  luaL_setmetatable(lua_State *L, const char *tname);
#else // Судя по всему, мы компилимся под JITом
#	include "luajit.h"
#	define CSSCRIPTS_HAS_JIT
#	define CSSCRIPTS_HAS_BIT
#	define _CSSCRIPTS_LIBVERSION LUAJIT_VERSION
#endif

#ifndef _CSSCRIPTS_LIBVERSION
#	define _CSSCRIPTS_LIBVERSION LUA_VERSION
#endif

#ifdef CSSCRIPTS_STATIC_LIBLINK
#	define CSSCRIPTS_LIBVERSION _CSSCRIPTS_LIBVERSION " [S]"
#else
#	define CSSCRIPTS_LIBVERSION _CSSCRIPTS_LIBVERSION
#endif

// 	Пути поиска C/Lua библиотек и запускаемых скриптов
#ifdef LUA_VERSION_MAJOR
#	define CSSCRIPTS_PATH_VER LUA_VERSION_MAJOR "." LUA_VERSION_MINOR PATH_DELIM
#elif LUA_VERSION_NUM == 501
#	define CSSCRIPTS_PATH_VER "5.1" PATH_DELIM
#endif

#define CSSCRIPTS_PATH_LDATA "." PATH_DELIM "luadata" PATH_DELIM
#define CSSCRIPTS_PATH_LROOT "." PATH_DELIM "lua" PATH_DELIM
#define CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_LROOT "clibs" PATH_DELIM
#define CSSCRIPTS_PATH_LOADALL "loadall." DLIB_EXT
#define CSSCRIPTS_PATH_CFILE "?." DLIB_EXT
#define CSSCRIPTS_PATH_LFILE "?.lua"
#define CSSCRIPTS_PATH_IFILE "?" PATH_DELIM "init.lua"
#define CSSCRIPTS_PATH_RSCRIPTS "." PATH_DELIM "scripts" PATH_DELIM
#define CSSCRIPTS_PATH_SCRIPTS CSSCRIPTS_PATH_RSCRIPTS CSSCRIPTS_PATH_VER

#ifdef CSSCRIPTS_PATH_VER
#	define CSSCRIPTS_PATH_LADD CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_LFILE ";" \
        CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_IFILE ";"
#	define CSSCRIPTS_PATH_CADD CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_CFILE ";" \
     	CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_LOADALL ";"
#else
#	define CSSCRIPTS_PATH_ADD ""
#endif

#define CSSCRIPTS_PATH CSSCRIPTS_PATH_LADD CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_LFILE ";" \
  	CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_IFILE
#define CSSCRIPTS_CPATH CSSCRIPTS_PATH_CADD CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_CFILE ";" \
	CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_LOADALL

#define lua_addnumconst(L, n) (lua_pushnumber(L, n), lua_setglobal(L, #n))
#define scr_addintconst(L, n) (lua_pushinteger(L, n), lua_setglobal(L, #n))

int scr_checktabfield(lua_State *L, int idx, const char *fname, int ftype);
int scr_checktabfieldud(lua_State *L, int idx, const char *fname, const char *meta);
void scr_createtype(lua_State *L, const char *meta, const luaL_Reg *meths);
#endif
