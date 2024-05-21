#pragma once

#include "tcp_connection.h"
#include "hashmap.h"


#define NB_MAX_CLIENTS 200
#define NB_MAX_MESSAGES 2000

typedef struct {
    // Channel name
    char name[MAX_NAME_LENGTH];

    // Pseudo of the client that created this channel
    char host[MAX_NAME_LENGTH];

    // List of authorized clients
    char clients[MAX_NAME_LENGTH][NB_MAX_CLIENTS];

    // Number of clients
    int nb_clients;

    // Liste of the lasts messages
    Message msgs[NB_MAX_MESSAGES];

    // Number of messages in this channel
    int nb_msgs;

    // Indicates that NB_MAX_MESSAGES messages have been exceeded in the chat room,
    //  therefore the periodicity of the msgs table is activated
	bool msgs_boucle;

} Channel;


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

