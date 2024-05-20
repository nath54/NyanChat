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

#include "../include/tcp_connection.h"
#include "../include/server.h"


// Fonction qui traite les messages reçus
void on_msg_received(TcpConnection* con, SOCKET sock,
                     Message msg, size_t msg_length,
                     void* custom_args)
{
    (void)con;
    (void)sock;
    (void)msg_length;
    ServerState* sstate = custom_args;
    (void)sstate;

    printf("Message reçu : \"%s\"\n", msg.msg);
}


void on_stdin_server(TcpConnection* con,
                     char msg[T_MSG_MAX], size_t msg_len,
                     void* custom_args)
{
    (void)con;
    (void)msg_len;
    ServerState* sstate = custom_args;
    (void)sstate;

    // On affiche juste l'entrée reçue
    printf("Message écrit : \"%s\"\n", msg);
}


void init_server_state(ServerState* sstate){
    (void)sstate;
    
    // TODO: compléter cette fonction
}

void free_server_state(ServerState* sstate){
    (void)sstate;

    // TODO: compléter cette fonction
}


// Main
int main(int argc, char **argv)
{
    ServerState server_state;
    TcpConnection con;

    // verification du nombre d'arguments sur la ligne de commande
    if(argc != 2)
    {
        printf("Usage: %s port_local\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    // Initialisation de la connection tcp du côté serveur
    init_server_state(&server_state);
    tcp_connection_server_init(&con, "127.0.0.1", port, 20, -1);

    // Boucle principale de la connection tcp
    tcp_connection_mainloop(&con,
                            on_msg_received, &server_state,
                            on_stdin_server, &server_state);

    // fermeture de la connection
    tcp_connection_close(&con);
    free_server_state(&server_state);

    return 0;
}
