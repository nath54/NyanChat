#pragma once

#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


// static noreturn void raler(const char *msg)
// {
//     perror(msg);
//     exit(1);
// }

#define raler(msg)                                                            \
    {                                                                         \
        perror(msg);                                                          \
        exit(EXIT_FAILURE);                                                   \
    }


#define CHK(op)                                                               \
    do                                                                        \
    {                                                                         \
        if ((op) == -1)                                                       \
            raler(#op);                                                       \
    } while (0)
#define CHKERRNO(op, code_to_be_different)                                    \
    do                                                                        \
    {                                                                         \
        if ((op) == -1 && errno != code_to_be_different)                      \
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

