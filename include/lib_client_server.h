#pragma once

#include "tcp_connection.h"

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


