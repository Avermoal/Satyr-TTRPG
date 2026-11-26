#ifndef NETWORKLEVEL_SERVER_H
#define NETWORKLEVEL_SERVER_H

#include <plibsys.h>
#include <gtk/gtk.h>

struct masterserver{
  int32_t plids;
  PSocket* tcp_socket;
  PSocket* udp_socket;
  PSocket* clients; // Must be ** not *
  PSocketAddress* addr;
  PSocketAddress* pladdrs; // Must be ** not *
  GMutex mutex;
};

/*Only for gtk-main thread section*/
struct masterserver* getserver();

void destroyserver(struct masterserver* ms);

/*Multy thread section*/
void serverpolling(struct masterserver* ms);

int32_t get_server_port(struct masterserver* ms);

char* get_server_port_in_text(struct masterserver* ms);

#endif/*NETWORKLEVEL_SERVER_H*/
