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

cs_bool lua_iscolor(scr_Context *L, int idx);
LuaColor *lua_newcolor(scr_Context *L);
LuaColor *lua_tocolor(scr_Context *L, int idx);
LuaColor *lua_checkcolor(scr_Context *L, int idx);
Color3 *lua_tocolor3(scr_Context *L, int idx);
Color3 *lua_checkcolor3(scr_Context *L, int idx);
Color4 *lua_tocolor4(scr_Context *L, int idx);
Color4 *lua_checkcolor4(scr_Context *L, int idx);

int luaopen_color(scr_Context *L);
#endif
