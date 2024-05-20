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
    (void)custom_args;
    printf("Message reçu : \"%s\"\n", msg.msg);
}


void on_stdin_server(TcpConnection* con,
                     char msg[T_MSG_MAX], size_t msg_len,
                     void* custom_args)
{
    (void)con;
    (void)msg_len;
    (void)custom_args;
    // On affiche juste l'entrée reçue
    printf("Message écrit : \"%s\"\n", msg);
}


// Main
int main(int argc, char **argv)
{
    
    TcpConnection con;

    // verification du nombre d'arguments sur la ligne de commande
    if(argc != 2)
    {
        printf("Usage: %s port_local\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    // Initialisation de la connection tcp du côté serveur
    tcp_connection_server_init(&con, "127.0.0.1", port, 20, -1);

    // Boucle principale de la connection tcp
    tcp_connection_mainloop(&con,
                            on_msg_received, NULL,
                            on_stdin_server, NULL);

    // fermeture de la connection
    tcp_connection_close(&con);

    return 0;
}
