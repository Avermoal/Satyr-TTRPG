#ifndef NETWORKLEVEL_CLIENT_H
#define NETWORKLEVEL_CLIENT_H

#include <stdint.h>

#include <plibsys.h>
#include <gtk/gtk.h>

struct gamestate;

struct playerclient{
  int32_t plid;
  char* plname;
  PSocket* tcp_socket;
  PSocket* udp_socket;
  PSocketAddress* addr;
  GMutex mutex;
};

struct playerclient* getclient(const char* ipport, const char* plname);

void destroyclient(struct playerclient* pc);

struct gamestate* client_get_game_state(struct playerclient* pc);

#endif/*NETWORKLEVEL_CLIENT_H*/
