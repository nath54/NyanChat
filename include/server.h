#pragma once

#include "tcp_connection.h"
#include "hashmap.h"


#define NB_MAX_CLIENTS 200
#define NB_MAX_MESSAGES 2000

typedef struct {
    // Nom du salon textuel
    char name[MAX_NAME_LENGTH];

    // Pseudo du créateur du channel
    char host[MAX_NAME_LENGTH];

    // Liste des clients autorisés
    char clients[MAX_NAME_LENGTH][NB_MAX_CLIENTS];

    // Nombre de clients
    int nb_clients;

    // liste des messages
    Message msgs[NB_MAX_MESSAGES];

    // Nombre de messages dans le salon
    int nb_msgs;

    // Indique qu’on a dépassé NB_MAX_MESSAGES messages dans le salon,
    //  donc on active la périodicité du tableau msgs
	bool msgs_boucle;

} Channel;


#define NB_MAX_CHANNELS 128
#define NB_MAX_CONNECTED_CLIENTS (MAX_POLL_SOCKETS - 4)

#define TIMEOUT_INACTIVE 30  // En secondes

typedef struct{
    bool connected;
    bool waiting_code_response;
    char* code_to_verify;
    char pseudo[MAX_NAME_LENGTH];
    time_t last_activity;
    int id_poll_socket_proxy;
} Client;


typedef struct{
    int nb_clients;

    Client* clients[NB_MAX_CONNECTED_CLIENTS];

    Hashmap* hm_pseudo_to_id;

} ServerState;

