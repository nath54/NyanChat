#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


#define BUFFER_SIZE 1024

#define MAX_POLL_SOCKETS 200

// Forward declaration
struct TcpConnection;

typedef struct TcpConnection{
    // Base TCP Socket Connection
    SOCKET sockfd;              // Descripteur du fichier du socket
    SOCKADDR_IN addr_recep;     // Adresse du recepteur
    SOCKADDR_IN addr_exped;     // Adresse de l'expediteur

    // Polling
    struct pollfd poll_fds[MAX_POLL_SOCKETS]; // Tableau des sockets du polling
    nfds_t nb_poll_fds;    // Nombre actuel de sockets de polling

    // Paramètres du serveur
    int timeout;    // Temps maximal d'inactivité avant fermeture du serveur
} TcpConnection;

typedef void(fn_on_msg)(TcpConnection* con, SOCKET sock,
                        char msg[BUFFER_SIZE], size_t msg_len);

static socklen_t sockaddr_size = sizeof(SOCKADDR_IN);



// Initialisation d'un socket pour un serveur tcp
void tcp_connection_init(TcpConnection* con,
                 char address_receptor[], int port_receptor,
                 int nb_max_connections_server,
                 int timeout_server);


// Boucle principale d'une connection tcp
void tcp_connection_mainloop(TcpConnection* con, fn_on_msg on_msg);



// Fermeture d'une connection tcp
void tcp_connection_close(TcpConnection* con);


