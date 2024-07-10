#ifndef CSLUACOLOR_H
#define CSLUACOLOR_H
#include <core.h>
#include <types/cpe.h>
#include "scripting.h"

typedef struct _LuaColor {
	cs_bool hasAlpha;
	union {
		Color3 c3;
		Color4 c4;
	} value;
} LuaColor;

cs_bool scr_iscolor(lua_State *L, int idx);
LuaColor *scr_newcolor(lua_State *L);
LuaColor *scr_tocolor(lua_State *L, int idx);
LuaColor *scr_checkcolor(lua_State *L, int idx);
Color3 *scr_tocolor3(lua_State *L, int idx);
Color3 *scr_checkcolor3(lua_State *L, int idx);
Color4 *scr_tocolor4(lua_State *L, int idx);
Color4 *scr_checkcolor4(lua_State *L, int idx);

int scr_libfunc(color)(lua_State *L);
#endif
