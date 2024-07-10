#include <core.h>
#include "scripting.h"

#if defined(CSSCRIPTING_BUILD_LUA)
	// Слой совместимости для чистой версии Lua 5.1
#	ifdef CSSCRIPTS_NONJIT_51
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
#	endif
#endif

#if LUA_VERSION_NUM < 503
	static int generic_tostring(lua_State *L) {
		cs_str typename = NULL;
		if(luaL_getmetafield(L, 1, "__name") && lua_isstring(L, -1))
			typename = lua_tostring(L, -1);
		else typename = luaL_typename(L, 1);

		lua_pushfstring(L, "%s: %p", typename, lua_touserdata(L, 1));
		return 1;
	}
#endif

void scr_createtype(lua_State *L, const char *meta, const luaL_Reg *meths) {
	if(!luaL_newmetatable(L, meta)) luaL_error(L, "Failed to create metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
#	if LUA_VERSION_NUM < 503
		lua_pushstring(L, meta);
		lua_setfield(L, -2, "__name");
		lua_pushcfunction(L, generic_tostring);
		lua_setfield(L, -2, "__tostring");
#	endif
	lua_pushstring(L, "Huh? Tf you doing here?");
	lua_setfield(L, -2, "__metatable");
	luaL_setfuncs(L, meths, 0);
	lua_pop(L, 1);
}
