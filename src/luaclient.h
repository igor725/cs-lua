#ifndef CSLUACLIENT_H
#define CSLUACLIENT_H
#include <types/client.h>
#include <lua.h>

Client *lua_checkclient(lua_State *L, int idx);
Client *lua_toclient(lua_State *L, int idx);
void lua_pushclient(lua_State *L, Client *client);
void lua_clearclient(lua_State *L, Client *client);
int luaopen_client(lua_State *L);
#endif
