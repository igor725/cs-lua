#ifndef SCRIPTING_H
#define SCRIPTING_H

#if defined(CSSCRIPTS_BUILD_LUA)
#	ifdef CORE_USE_WINDOWS
#		define LUA_BUILD_AS_DLL
#		define LUA_LIB
#	endif

#	include <lua.h>
#	include <lauxlib.h>
#	include <lualib.h>

#	define scr_Context lua_State
#	define scr_Number lua_Number
#	define scr_Integer lua_Integer
#	define scr_RegFuncs luaL_Reg

#	define scr_contextcreate() luaL_newstate()
#	define scr_contextclose(C) lua_close(C)
#	define scr_contextusage(C) lua_gc(C, LUA_GCCOUNT, 0)

#	define scr_libfunc(N) luaopen_##N

#	define scr_stacktop(C) lua_gettop(C)
#	define scr_stackpop(C, N) lua_pop(C, N)
#	define scr_stackrem(C, I) lua_remove(C, I)
#	define scr_stackpush(C, I) lua_pushvalue(C, I)
#	define scr_stackset(C, I) lua_settop(L, I)
#	define scr_stackcheck(C, N) lua_checkstack(C, N)

#	define scr_error(C) lua_error(C)
#	define scr_fmterror(C, Fm, ...) luaL_error(C, Fm, ##__VA_ARGS__)
#	define scr_argerror(C, I, E) luaL_argerror(C, I, E)
#	define scr_argassert(C, E, I, T) luaL_argcheck(C, E, I, T)

#	define scr_newntable(C, A, R) lua_createtable(C, A, R)
#	define scr_newtable(C) scr_newntable(C, 0, 0)
#	define scr_settabfield(C, T, F) lua_setfield(C, T, F)
#	define scr_gettabfield(C, T, F) lua_getfield(C, T, F)
#	define scr_getmetafield(C, I, F) luaL_getmetafield(C, I, F)
#	define scr_getfromtable(C, I) lua_gettable(C, I)
#	define scr_settotable(C, I) lua_settable(C, I)
#	define scr_gettablen(C, I) lua_objlen(C, I)

#	define scr_newlib(C, F) luaL_newlib(C, F)

#	define scr_allocmem(C, S) lua_newuserdata(C, S)
#	define scr_getmem(C, I) lua_touserdata(C, I)
#	define scr_setmemtype(C, T) luaL_setmetatable(C, T)
#	define scr_testmemtype(C, I, T) luaL_testudata(C, I, T)
#	define scr_checkmemtype(C, I, T) luaL_checkudata(C, I, T)

#	define scr_pushboolean(C, B) lua_pushboolean(C, B)
#	define scr_toboolean(C, I) ((cs_bool)lua_toboolean(C, I))

#	define scr_optnumber(C, I, D) luaL_optnumber(C, I, D)
#	define scr_pushnumber(C, N) lua_pushnumber(C, N)
#	define scr_checknumber(C, N) luaL_checknumber(C, N)
#	define scr_tonumber(C, N) lua_tonumber(C, N)

#	define scr_optinteger(C, I, D) luaL_optinteger(C, I, D)
#	define scr_pushinteger(C, I) lua_pushinteger(C, I)
#	define scr_checkinteger(C, I) luaL_checkinteger(C, I)
#	define scr_tointeger(C, I) lua_tointeger(C, I)

#	define scr_pushformatstring(C, Fm, ...) lua_pushfstring(C, Fm, ##__VA_ARGS__)
#	define scr_pushstring(C, S) lua_pushstring(C, S)
#	define scr_pushlstring(C, S, L) lua_pushlstring(C, S, L)
#	define scr_checkstring(C, I) luaL_checkstring(C, I)
#	define scr_tostring(C, I) lua_tostring(C, I)

#	define scr_pushnativefunc(C, Fn) lua_pushcfunction(C, Fn)
#	define scr_unprotectedcall(C, A, R) lua_call(C, A, R)
#	define scr_protectedcall(C, A, R, E) lua_pcall(C, A, R, E)

#	define scr_raweq(C, I1, I2) lua_rawequal(C, I1, I2)
#	define scr_rawset(C, I) lua_rawset(C, I)
#	define scr_rawseti(C, I, Kn) lua_rawseti(C, I, Kn)
#	define scr_rawgeti(C, I, Kn) lua_rawgeti(C, I, Kn)

#	define scr_pushnull(C) lua_pushnil(C)
#	define scr_pushptr(C, P) lua_pushlightuserdata(C, P)

#	define scr_isboolean(C, I) lua_isboolean(C, I)
#	define scr_istable(C, I) lua_istable(C, I)
#	define scr_isstring(C, I) lua_isstring(C, I)
#	define scr_isinteger(C, I) lua_isinteger(C, I)
#	define scr_isnumber(C, I) lua_isnumber(C, I)
#	define scr_ismem(C, I) lua_isuserdata(C, I)
#	define scr_isptr(C, I) lua_islightuserdata(C, I)
#	define scr_isnull(C, I) lua_isnil(C, I)
#	define scr_isnativefunc(C, I) lua_iscfunction(C, I)
#	define scr_isfunc(C, I) lua_isfunction(C, I)
#	define scr_isempty(C, I) lua_isnone(L, I)
#	define scr_isemptyornull(C, I) lua_isnoneornil(C, I)

#	define scr_istype(C, I, T) (lua_type(L, I) == T)
#	define scr_typename(C, T) lua_typename(C, T)

// Обеспечиваем совместимость с большинством версий Lua
#	if LUA_VERSION_NUM < 503 // Компилимся под чем-ито ниже Lua 5.3
#		define lua_absindex(L, idx) ((idx) > 0 || ((idx) <= LUA_REGISTRYINDEX) ? (idx) : scr_stacktop(L) + (idx) + 1)
#	endif

#	if LUA_VERSION_NUM < 501
// Кто-то вообще будет пытаться собрать плагин с этими версиями?
#		error "This version of Lua is not supported"
#	elif LUA_VERSION_NUM > 501 // Что-то старше 5.1
#		define lua_objlen(L, idx) lua_rawlen(L, idx)
#		if LUA_VERSION_NUM == 502 || defined(LUA_COMPAT_BITLIB)
#			define luaopen_bit luaopen_bit32
#			define CSSCRIPTS_HAS_BIT
#		endif
#	elif !defined(LUA_JITLIBNAME) // Чистый Lua 5.1
#		define CSSCRIPTS_NONJIT_51
//		Немного приколов из Lua 5.2
#		define scr_newlibtable(L, l) \
		scr_newntable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)
#		define luaL_newlib(L, l)	(scr_newlibtable(L, l), luaL_setfuncs(L, l, 0))

		void  luaL_setfuncs(scr_Context *L, const scr_RegFuncs *l, int nup);
		void *scr_testmemtype(scr_Context *L, int ud, const char *tname);
		void  scr_setmemtype(scr_Context *L, const char *tname);
#	else // Судя по всему, мы компилимся под JITом
#		include "luajit.h"
#		define CSSCRIPTS_HAS_JIT
#		define CSSCRIPTS_HAS_BIT
#		define _CSSCRIPTS_LIBVERSION LUAJIT_VERSION
#	endif

#	ifndef _CSSCRIPTS_LIBVERSION
#		define _CSSCRIPTS_LIBVERSION LUA_VERSION
#	endif

#	ifdef CSSCRIPTS_STATIC_LIBLINK
#		define CSSCRIPTS_LIBVERSION _CSSCRIPTS_LIBVERSION " [S]"
#	else
#		define CSSCRIPTS_LIBVERSION _CSSCRIPTS_LIBVERSION
#	endif

// 	Пути поиска C/Lua библиотек и запускаемых скриптов
#	ifdef LUA_VERSION_MAJOR
#		define CSSCRIPTS_PATH_VER LUA_VERSION_MAJOR "." LUA_VERSION_MINOR PATH_DELIM
#	elif LUA_VERSION_NUM == 501
#		define CSSCRIPTS_PATH_VER "5.1" PATH_DELIM
#	endif

#	define CSSCRIPTS_PATH_LDATA "." PATH_DELIM "luadata" PATH_DELIM
#	define CSSCRIPTS_PATH_LROOT "." PATH_DELIM "lua" PATH_DELIM
#	define CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_LROOT "clibs" PATH_DELIM
#	define CSSCRIPTS_PATH_LOADALL "loadall." DLIB_EXT
#	define CSSCRIPTS_PATH_CFILE "?." DLIB_EXT
#	define CSSCRIPTS_PATH_LFILE "?.lua"
#	define CSSCRIPTS_PATH_IFILE "?" PATH_DELIM "init.lua"
#	define CSSCRIPTS_PATH_RSCRIPTS "." PATH_DELIM "scripts" PATH_DELIM
#	define CSSCRIPTS_PATH_SCRIPTS CSSCRIPTS_PATH_RSCRIPTS CSSCRIPTS_PATH_VER

#	ifdef CSSCRIPTS_PATH_VER
#		define CSSCRIPTS_PATH_LADD CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_LFILE ";" \
    	       CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_IFILE ";"
#		define CSSCRIPTS_PATH_CADD CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_CFILE ";" \
     	      CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_VER CSSCRIPTS_PATH_LOADALL ";"
#	else
#		define CSSCRIPTS_PATH_ADD ""
#	endif

#	define CSSCRIPTS_PATH CSSCRIPTS_PATH_LADD CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_LFILE ";" \
  	      CSSCRIPTS_PATH_LROOT CSSCRIPTS_PATH_IFILE
#	define CSSCRIPTS_CPATH CSSCRIPTS_PATH_CADD CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_CFILE ";" \
    	    CSSCRIPTS_PATH_CROOT CSSCRIPTS_PATH_LOADALL

#	define lua_addnumconst(L, n) (scr_pushnumber(L, n), lua_setglobal(L, #n))
#	define scr_addintconst(L, n) (scr_pushinteger(L, n), lua_setglobal(L, #n))

#elif defined(CSSCRIPTS_BUILD_PYTHON)
#	error Python scripting backend not yet implemented
#elif defined(CSSCRIPTS_BUILD_DUKTAPE)
#		error JavaScript scripting backend not yet implemented
#else
#	error Unknown scripting backend
#endif

int scr_checktabfield(scr_Context *L, int idx, const char *fname, int ftype);
int scr_checktabfieldud(scr_Context *L, int idx, const char *fname, const char *meta);
void scr_createtype(scr_Context *L, const char *meta, const scr_RegFuncs *meths);
#endif
