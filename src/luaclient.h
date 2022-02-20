#ifndef CSLUACLIENT_H
#define CSLUACLIENT_H
#include <core.h>
#include <client.h>
#include "luaplugin.h"

Client *lua_checkclient(lua_State *L, int idx);
void lua_pushclient(lua_State *L, Client *client);
void lua_clearclient(lua_State *L, Client *client);
int luaopen_client(lua_State *L);
#endif
