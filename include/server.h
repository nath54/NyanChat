#pragma once

#include "lib_client_server.h"
#include "hashmap.h"


#define NB_MAX_CHANNELS 128
#define NB_MAX_CONNECTED_CLIENTS (MAX_POLL_SOCKETS - 4)

#define TIMEOUT_INACTIVE 30  // In seconds

typedef struct{
    bool connected;
    char pseudo[MAX_NAME_LENGTH];
    time_t last_activity;
    int id_poll_socket_proxy;
} Client;


typedef struct{
    int nb_clients;

    Client* clients[NB_MAX_CONNECTED_CLIENTS];

    Hashmap* hm_pseudo_to_id;

} ServerState;

