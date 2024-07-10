#ifndef CSLUACLIENT_H
#define CSLUACLIENT_H
#include <types/client.h>
#include "scripting.h"

Client *scr_checkclient(lua_State *L, int idx);
Client *scr_toclient(lua_State *L, int idx);
void scr_pushclient(lua_State *L, Client *client);
void scr_clearclient(lua_State *L, Client *client);

int scr_libfunc(client)(lua_State *L);
#endif
