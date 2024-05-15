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


// Fonction qui traite les messages reçus
void on_msg_received(TcpConnection* con, SOCKET sock,
                     char msg[BUFFER_SIZE], size_t msg_length)
{
    // On affiche juste le message reçu   
    if(msg[BUFFER_SIZE-1] != '\0'){
        char last_c = msg[BUFFER_SIZE-1];
        msg[BUFFER_SIZE-1] = '\0';
        printf("Message reçu : \"%s%c\"\n", msg, last_c);
        msg[BUFFER_SIZE-1] = last_c;
    }
    else{
        printf("Message reçu : \"%s\"\n", msg);
    }
}



// Main
int main(int argc, char **argv)
{
    
    TcpConnection con;

    // verification du nombre d'arguments sur la ligne de commande
    if(argc != 2)
    {
        printf("Usage: %s port_local\n", argv[0]);
        exit(-1);
    }

    int port = atoi(argv[1]);

    // Initialisation de la connection tcp du côté serveur
    tcp_connection_init(&con, "127.0.0.1", port, 20, -1);

    // Boucle principale de la connection tcp
    tcp_connection_mainloop(&con, on_msg_received);

    // fermeture de la connection
    tcp_connection_close(&con);

    return 0;
}
