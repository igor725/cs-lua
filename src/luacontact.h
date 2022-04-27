#ifndef LUACONTACT_H
#define LUACONTACT_H
#include "luascript.h"

#define CSLUA_CONTACT_NAMELEN   32
#define CSLUA_CONTACT_MAXSCRIPTS 16
#define CSLUA_CONTACT_MAX       128

int luaopen_contact(lua_State *L);
#endif
