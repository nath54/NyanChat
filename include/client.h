#pragma once

#include <stdbool.h>
#include "../include/tcp_connection.h"


typedef struct {
    bool connected;
    char pseudo[T_NOM_MAX];
} ClientState;
