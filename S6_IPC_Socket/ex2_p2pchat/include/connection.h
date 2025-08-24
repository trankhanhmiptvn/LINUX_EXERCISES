#ifndef CONNECTION_H
#define CONNECTION_H

#include "common.h"

PeerConn* add_connection(int sockfd, struct sockaddr_in *peer_addr);
void* conn_recv_thread(void *arg);
void reap_connections();
PeerConn* find_connection_by_id(int id);
void remove_node_locked(PeerConn *prev, PeerConn *cur);

#endif