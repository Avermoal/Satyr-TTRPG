#include "NetworkLevel/server.h"

#include <stdlib.h>

/*Only for gtk-main thread section*/
struct masterserver* getserver()
{
  struct masterserver* ms = (struct masterserver*)calloc(1, sizeof(struct masterserver));
  if(!ms){
    return nullptr;
  }
  ms->plids = -1;
  /*Address local host on any port*/
  ms->addr = p_socket_address_new("127.0.0.1", 34534);
  if(!ms->addr){
    free(ms);
    return nullptr;
  }
  /*TCP socket create*/
  ms->tcp_socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, nullptr);
  if(!ms->tcp_socket){
    p_socket_address_free(ms->addr);
    free(ms);
    return nullptr;
  }
  /*Bind socket to local host address*/
  if(p_socket_bind(ms->tcp_socket, ms->addr, true, nullptr) == false){
    p_socket_free(ms->tcp_socket);
    p_socket_address_free(ms->addr);
    free(ms);
    return nullptr;
  }
  /*Mutex init*/
  g_mutex_init(&ms->mutex);
  return ms;
}

void destroyserver(struct masterserver* ms)
{
  if(!ms){
    return;
  }
  if(ms->tcp_socket){
    p_socket_free(ms->tcp_socket);
    ms->tcp_socket = nullptr;
  }
  if(ms->udp_socket){
    p_socket_free(ms->udp_socket);
    ms->udp_socket = nullptr;
  }
  if(ms->clients){
    p_socket_free(ms->clients);
    ms->clients = nullptr;
  }
  if(ms->addr){
    p_socket_address_free(ms->addr);
    ms->addr = nullptr;
  }
  g_mutex_clear(&ms->mutex);
  ms->plids = -1;
  free(ms);
}

/*Multy thread section*/
void serverpolling(struct masterserver* ms)
{
  g_mutex_lock(&ms->mutex);
  /*Get new connections*/
  if(p_socket_io_condition_wait(ms->tcp_socket, P_SOCKET_IO_CONDITION_POLLIN, nullptr)){
    if(p_socket_listen(ms->tcp_socket, nullptr)){
      ms->clients = p_socket_accept(ms->tcp_socket, nullptr);
      ms->plids++;
      char plid[16];
      snprintf(plid, sizeof(plid), "%d", ms->plids);
      p_socket_send(ms->clients, plid, sizeof(plid) - 1, nullptr);
    }
  }
  g_mutex_unlock(&ms->mutex);
}

int32_t get_server_port(struct masterserver* ms)
{
  g_mutex_lock(&ms->mutex);
  int32_t ret = p_socket_address_get_port(ms->addr);
  g_mutex_unlock(&ms->mutex);
  return ret;
}

char* get_server_port_in_text(struct masterserver* ms)
{
  g_mutex_lock(&ms->mutex);
  int32_t port = p_socket_address_get_port(ms->addr);
  char* ret = g_strdup_printf("%d", port);
  g_mutex_unlock(&ms->mutex);
  return ret;
}
