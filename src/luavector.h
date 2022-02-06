#ifndef CSLUAVECTOR_H
#define CSLUAVECTOR_H
#include <core.h>
#include <vector.h>
#include "luaplugin.h"

typedef struct LuaVector {
	cs_int8 type;
	union {
		Vec f;
		SVec s;
	} value;
} LuaVector;

LuaVector *lua_checkvector(lua_State *L, int idx);
int luaopen_vector(lua_State *L);
#endif
