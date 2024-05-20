#pragma once

#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define CHK(op)                                                               \
    do                                                                        \
    {                                                                         \
        if ((op) == -1)                                                       \
            raler(#op);                                                       \
    } while (0)
#define CHKN(op)                                                              \
    do                                                                        \
    {                                                                         \
        if ((op) == NULL)                                                     \
            raler(#op);                                                       \
    } while (0)
#define TCHK(op)                                                              \
    do                                                                        \
    {                                                                         \
        if ((errno = (op)) > 0)                                               \
            raler(#op);                                                       \
    } while (0)



static noreturn void raler(const char *msg)
{
    perror(msg);
    exit(1);
}


