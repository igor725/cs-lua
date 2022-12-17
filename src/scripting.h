#ifndef SCRIPTING_H
#define SCRIPTING_H

#if defined(CSSCRIPTS_BUILD_LUA)
#	include <lua.h>
#	include <lauxlib.h>
#	include <lualib.h>

#	define scr_Context lua_State
#elif defined(CSSCRIPTS_BUILD_PYTHON)
#	error Not yet implemented
#else
#	error Unknown scripting backend
#endif

#endif
