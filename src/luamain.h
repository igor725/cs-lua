#ifndef CSLUAMAIN_H
#define CSLUAMAIN_H
#include <core.h>
#include <types/list.h>
#include "luascript.h"

extern AListField *headScript;

INL static LuaScript *getscriptptr(AListField *field) {
	return (LuaScript *)AList_GetValue(field).ptr;
}
#endif
