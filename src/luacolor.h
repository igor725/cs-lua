#ifndef CSLUACOLOR_H
#define CSLUACOLOR_H
#include <core.h>
#include <types/cpe.h>
#include "luascript.h"

typedef struct _LuaColor {
	cs_bool hasAlpha;
	union {
		Color3 c3;
		Color4 c4;
	} value;
} LuaColor;

LuaColor *lua_newcolor(lua_State *L);
LuaColor *lua_checkcolor(lua_State *L, cs_int32 idx);
Color3 *lua_checkcolor3(lua_State *L, cs_int32 idx);
Color4 *lua_checkcolor4(lua_State *L, cs_int32 idx);

int luaopen_color(lua_State *L);
#endif
