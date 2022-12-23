#include <core.h>
#include "scripting.h"

#if defined(CSSCRIPTING_BUILD_LUA)
	// Слой совместимости для чистой версии Lua 5.1
#	ifdef CSSCRIPTS_NONJIT_51
		void luaL_setfuncs(scr_Context *L, const scr_RegFuncs *l, int nup) {
			for(; l->name != NULL; l++) {
				int i;
				for(i = 0; i < nup; i++)
					scr_stackpush(L, -nup);
				lua_pushcclosure(L, l->func, nup);
				scr_settabfield(L, -(nup + 2), l->name);
			}
			scr_stackpop(L, nup);
		}

		void scr_setmemtype(scr_Context *L, const char *tname) {
			luaL_getmetatable(L, tname);
			lua_setmetatable(L, -2);
		}

		void *scr_testmemtype (scr_Context *L, int ud, const char *tname) {
			void *p = scr_getmem(L, ud);
			if(p != NULL) {
				if(lua_getmetatable(L, ud)) {
					luaL_getmetatable(L, tname);
					if(!scr_raweq(L, -1, -2))
						p = NULL;
					scr_stackpop(L, 2);
					return p;
				}
			}
			return NULL;
		}
#	endif
#endif

#if LUA_VERSION_NUM < 503
	static int generic_tostring(scr_Context *L) {
		cs_str typename = NULL;
		if(scr_getmetafield(L, 1, "__name") && scr_isstring(L, -1))
			typename = scr_tostring(L, -1);
		else typename = luaL_typename(L, 1);

		scr_pushformatstring(L, "%s: %p", typename, scr_getmem(L, 1));
		return 1;
	}
#endif

void scr_createtype(scr_Context *L, const char *meta, const scr_RegFuncs *meths) {
	if(!luaL_newmetatable(L, meta)) scr_fmterror(L, "Failed to create metatable");
	scr_stackpush(L, -1);
	scr_settabfield(L, -2, "__index");
#	if LUA_VERSION_NUM < 503
		scr_pushstring(L, meta);
		scr_settabfield(L, -2, "__name");
		scr_pushnativefunc(L, generic_tostring);
		scr_settabfield(L, -2, "__tostring");
#	endif
	scr_pushstring(L, "Huh? Tf you doing here?");
	scr_settabfield(L, -2, "__metatable");
	luaL_setfuncs(L, meths, 0);
	scr_stackpop(L, 1);
}
