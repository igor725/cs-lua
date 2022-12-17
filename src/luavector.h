#ifndef CSLUAVECTOR_H
#define CSLUAVECTOR_H
#include <core.h>
#include <vector.h>
#include "scripting.h"

typedef enum _ELuaVectorType {
	LUAVECTOR_TFLOAT = 0,
	LUAVECTOR_TSHORT
} ELuaVectorType;

typedef struct _LuaVector {
	ELuaVectorType type;
	union {
		Vec f;
		SVec s;
	} value;
} LuaVector;

cs_bool lua_isvector(scr_Context *L, int idx);
LuaVector *lua_newvector(scr_Context *L);
LuaVector *lua_checkvector(scr_Context *L, int idx);
LuaVector *lua_tovector(scr_Context *L, int idx);
Vec *lua_checkfloatvector(scr_Context *L, int idx);
Vec *lua_tofloatvector(scr_Context *L, int idx);
SVec *lua_checkshortvector(scr_Context *L, int idx);
SVec *lua_toshortvector(scr_Context *L, int idx);
int luaopen_vector(scr_Context *L);
#endif
