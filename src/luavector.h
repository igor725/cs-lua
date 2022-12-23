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

cs_bool scr_isvector(scr_Context *L, int idx);
LuaVector *scr_newvector(scr_Context *L);
LuaVector *scr_checkvector(scr_Context *L, int idx);
LuaVector *scr_tovector(scr_Context *L, int idx);
Vec *scr_checkfloatvector(scr_Context *L, int idx);
Vec *scr_tofloatvector(scr_Context *L, int idx);
SVec *scr_checkshortvector(scr_Context *L, int idx);
SVec *scr_toshortvector(scr_Context *L, int idx);

int scr_libfunc(vector)(scr_Context *L);
#endif
