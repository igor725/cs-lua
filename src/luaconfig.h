#ifndef CSLUACONFIG_H
#define CSLUACONFIG_H
#include <types/config.h>
#include "scripting.h"

CStore *scr_checkcfgstore(scr_Context *L, int idx);

int scr_libfunc(config)(scr_Context *L);
#endif
