#ifndef LUACONTACT_H
#define LUACONTACT_H
#include "scripting.h"

#define CSLUA_CONTACT_NAMELEN   32
#define CSLUA_CONTACT_MAXSCRIPTS 16
#define CSLUA_CONTACT_MAX       128

int luaopen_contact(scr_Context *L);
#endif
