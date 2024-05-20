#pragma once

#include <stdbool.h>
#include "tcp_connection.h"


typedef struct {
    bool connected;
    char pseudo[T_NOM_MAX];
    bool waiting_pseudo_confirmation;

    // Salon actuel
    int type_current_dest; // 0=salon par défaut, 1=salon privé, 2=msg_privé
    // Nom du salon où il est, ou pseudo du client de msg privé
    char destination[T_NOM_MAX];

    // Liste de messages envoyés qui attendent un acquittement
    //  à initialiser au début du client, et à libérer à la fin
    Message* msg_waiting_acq;
    size_t tot_msg_waiting_acq;
    size_t nb_msg_waiting_acq;

} ClientState;
