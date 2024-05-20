#pragma once

#include "../include/tcp_connection.h"

#define NB_MAX_CLIENTS 200
#define NB_MAX_MESSAGES 2000

typedef struct {
    // Nom du salon textuel
    char name[T_NOM_MAX];

    // Pseudo du créateur du channel
    char host[T_NOM_MAX];

    // Liste des clients autorisés
    char clients[T_NOM_MAX][NB_MAX_CLIENTS];

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


typedef struct{
    
    // TODO : compléter cette structure

} ServerState;

