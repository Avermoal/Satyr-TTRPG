#include "NetworkLevel/client.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "GameState/game_state.h"

struct playerclient* getclient(const char* ipport, const char* plname)
{
  struct playerclient* pc = (struct playerclient*)calloc(1, sizeof(struct playerclient));
  if(!pc){
    return nullptr;
  }
  /*Parse ipport <ip.ip.ip.ip:port>*/
  char sep = ':';
  char ip[16];
  char port[16];
  const char* pos = strchr(ipport, sep);
  if(pos != nullptr){
    size_t len = (size_t)(pos - ipport);
    if(len >= sizeof(ip)){
      len = sizeof(ip) - 1;
    }
    memcpy(ip, ipport, len);
    ip[len] = '\0';
    if(*(pos + 1) != '\0'){
      snprintf(port, sizeof(port), "%s", pos + 1);
    }else{
      free(pc);
      return nullptr;
    }
  }
  /*TCP socket create*/
  int32_t portint = strtol(port, nullptr, 10);
  if(errno == ERANGE){
    free(pc);
    return nullptr;
  }
  pc->addr = p_socket_address_new(ip, portint);
  if(!pc->addr){
    free(pc);
    return nullptr;
  }
  pc->tcp_socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, nullptr);
  if(!pc->tcp_socket){
    p_socket_address_free(pc->addr);
    free(pc);
    return nullptr;
  }
  /*Connect to server (DM)*/
  if(!p_socket_connect(pc->tcp_socket, pc->addr, nullptr)){
    p_socket_address_free(pc->addr);
    p_socket_free(pc->tcp_socket);
    free(pc);
    return nullptr;
  }
  /*Get players id by TCP*/
  char plidbuf[16];
  pssize received = p_socket_receive(pc->tcp_socket, plidbuf, sizeof(plidbuf) - 1, nullptr);
  if(received <= 0){
    p_socket_address_free(pc->addr);
    p_socket_free(pc->tcp_socket);
    free(pc);
    return nullptr;
  }
  plidbuf[received] = '\0';
  int32_t plid = strtol(plidbuf, nullptr, 10);
  if(errno == ERANGE){
    p_socket_address_free(pc->addr);
    p_socket_free(pc->tcp_socket);
    free(pc);
    return nullptr;
  }
  pc->plid = plid;
  pc->plname = strdup(plname);
  if(!pc->plname){
    p_socket_address_free(pc->addr);
    p_socket_free(pc->tcp_socket);
    free(pc);
    return nullptr;
  }
  g_mutex_init(&pc->mutex);
  return pc;
}

void destroyclient(struct playerclient* pc)
{
  if(!pc){
    return;
  }
  if(pc->plname){
    free(pc->plname);
    pc->plname = nullptr;
  }
  if(pc->tcp_socket){
    p_socket_free(pc->tcp_socket);
    pc->tcp_socket = nullptr;
  }
  if(pc->udp_socket){
    p_socket_free(pc->udp_socket);
    pc->udp_socket = nullptr;
  }
  if(pc->addr){
    p_socket_address_free(pc->addr);
    pc->addr = nullptr;
  }
  g_mutex_clear(&pc->mutex);
  pc->plid = -1;
  free(pc);
}

struct gamestate* client_get_game_state(struct playerclient* pc)
{
  return nullptr;
}
