#ifndef CSLUACLIENT_H
#define CSLUACLIENT_H
#include <types/client.h>
#include "scripting.h"

Client *scr_checkclient(scr_Context *L, int idx);
Client *scr_toclient(scr_Context *L, int idx);
void scr_pushclient(scr_Context *L, Client *client);
void scr_clearclient(scr_Context *L, Client *client);

int scr_libfunc(client)(scr_Context *L);
#endif
