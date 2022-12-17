#ifndef CSLUACLIENT_H
#define CSLUACLIENT_H
#include <types/client.h>
#include "scripting.h"

Client *lua_checkclient(scr_Context *L, int idx);
Client *lua_toclient(scr_Context *L, int idx);
void lua_pushclient(scr_Context *L, Client *client);
void lua_clearclient(scr_Context *L, Client *client);
int luaopen_client(scr_Context *L);
#endif
