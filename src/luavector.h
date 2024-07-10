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

cs_bool scr_isvector(lua_State *L, int idx);
LuaVector *scr_newvector(lua_State *L);
LuaVector *scr_checkvector(lua_State *L, int idx);
LuaVector *scr_tovector(lua_State *L, int idx);
Vec *scr_checkfloatvector(lua_State *L, int idx);
Vec *scr_tofloatvector(lua_State *L, int idx);
SVec *scr_checkshortvector(lua_State *L, int idx);
SVec *scr_toshortvector(lua_State *L, int idx);

int scr_libfunc(vector)(lua_State *L);
#endif
