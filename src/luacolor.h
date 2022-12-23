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

cs_bool scr_iscolor(scr_Context *L, int idx);
LuaColor *scr_newcolor(scr_Context *L);
LuaColor *scr_tocolor(scr_Context *L, int idx);
LuaColor *scr_checkcolor(scr_Context *L, int idx);
Color3 *scr_tocolor3(scr_Context *L, int idx);
Color3 *scr_checkcolor3(scr_Context *L, int idx);
Color4 *scr_tocolor4(scr_Context *L, int idx);
Color4 *scr_checkcolor4(scr_Context *L, int idx);

int scr_libfunc(color)(scr_Context *L);
#endif
