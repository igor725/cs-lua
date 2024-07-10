#ifndef LUACONTACT_H
#define LUACONTACT_H
#include "scripting.h"

#define CSSCRIPTS_CONTACT_NAMELEN   32
#define CSSCRIPTS_CONTACT_MAXSCRIPTS 16
#define CSSCRIPTS_CONTACT_MAX       128

int scr_libfunc(contact)(lua_State *L);
#endif
