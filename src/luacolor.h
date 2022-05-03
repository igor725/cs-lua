#ifndef CSLUACOLOR_H
#define CSLUACOLOR_H
#include <core.h>
#include <types/cpe.h>
#include <lua.h>

typedef struct _LuaColor {
	cs_bool hasAlpha;
	union {
		Color3 c3;
		Color4 c4;
	} value;
} LuaColor;

cs_bool lua_iscolor(lua_State *L, int idx);
LuaColor *lua_newcolor(lua_State *L);
LuaColor *lua_tocolor(lua_State *L, int idx);
LuaColor *lua_checkcolor(lua_State *L, int idx);
Color3 *lua_tocolor3(lua_State *L, int idx);
Color3 *lua_checkcolor3(lua_State *L, int idx);
Color4 *lua_tocolor4(lua_State *L, int idx);
Color4 *lua_checkcolor4(lua_State *L, int idx);

int luaopen_color(lua_State *L);
#endif
