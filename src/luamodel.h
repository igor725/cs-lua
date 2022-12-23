#ifndef CSLUAMODEL_H
#define CSLUAMODEL_H
#include <types/cpe.h>
#include "scripting.h"

CPEModel *scr_checkmodel(scr_Context *L, int idx);

int scr_libfunc(model)(scr_Context *L);
#endif
