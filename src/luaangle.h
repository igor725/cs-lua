#ifndef CSLUAANGLE_H
#define CSLUAANGLE_H
#include <core.h>
#include <vector.h>
#include "scripting.h"

cs_bool scr_isangle(scr_Context *L, int idx);
Ang *scr_newangle(scr_Context *L);
Ang *scr_checkangle(scr_Context *L, int idx);
Ang *scr_toangle(scr_Context *L, int idx);

int scr_libfunc(angle)(scr_Context *L);
#endif
