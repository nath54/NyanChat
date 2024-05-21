#pragma once

#include <stdbool.h>
#include "tcp_connection.h"


typedef struct {
    bool connected;
    char pseudo[MAX_NAME_LENGTH];
    bool waiting_pseudo_confirmation;

    char* connection_code;

    // Current place type to send messages
    // 0=default channel, 1=private channel, 2=private discussion
    int type_current_dest;
    // Current place name, pseudo or channel name
    char destination[MAX_NAME_LENGTH];

    // List of sent messages that are waiting for an acknowledgement
    Message* msg_waiting_ack;
    size_t tot_msg_waiting_ack;
    size_t nb_msg_waiting_ack;

} ClientState;
