#ifndef CSLUAVECTOR_H
#define CSLUAVECTOR_H
#include <core.h>
#include <vector.h>
#include "luascript.h"

typedef struct LuaVector {
	cs_int8 type;
	union {
		Vec f;
		SVec s;
	} value;
} LuaVector;

LuaVector *lua_newvector(lua_State *L);
LuaVector *lua_checkvector(lua_State *L, int idx);
Vec *lua_checkfloatvector(lua_State *L, int idx);
SVec *lua_checkshortvector(lua_State *L, int idx);
int luaopen_vector(lua_State *L);
#endif
