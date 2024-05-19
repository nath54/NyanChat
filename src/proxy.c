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
#include <unistd.h>
#include <stdnoreturn.h>

#include <pthread.h>
#include <semaphore.h>

#include "../include/tcp_connection.h"


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



noreturn void raler(const char *msg)
{
    perror(msg);
    exit(1);
}




/* ------------------ VARIABLES GLOBALES ------------------ */

sem_t semaphore;
int etat_server;

#define SERVER_OK 1
#define SERVER_WAIT_CONNECTION 0
#define SERVER_ERROR -1

char* ip_server;
int port_server;
int port_clients;

TcpConnection con_server;
TcpConnection con_clients;

/* ------------------ FIN VARIABLES GLOBALES ------------------*/



void on_client_received(TcpConnection* con, SOCKET sock,
                        Message msg, size_t msg_length)
{
    (void)con;
    (void)sock;
    (void)msg_length;
    tcp_connection_send_struct_message(&con_server, con_server.poll_fds[0].fd,
                                       msg);
}


void on_server_received(TcpConnection* con, SOCKET sock,
                        Message msg, size_t msg_length)
{
    (void)con;
    (void)sock;
    (void)msg;
    (void)msg_length;
    // TODO : récupérer le SOCKET DU CLIENT
}


void* gestion_server(void* arg)
{
    (void)arg;
    // Initialisation de la connection qui va écouter le serveur
    tcp_connection_client_init(&con_server, ip_server, port_server, -1);

    // Boucle principale de la connection tcp qui écoute le serveur
    tcp_connection_mainloop(&con_server, on_server_received, NULL);

    // fermeture de la connection qui a écouté le serveur
    tcp_connection_close(&con_server);

    return NULL;
}


void* gestion_clients(void* arg)
{
    (void)arg;
    // Initialisation de la connection qui va écouter les clients
    tcp_connection_server_init(&con_clients, "127.0.0.1", port_clients, 20, -1);

    // Boucle principale de la connection tcp qui écoute les clients
    tcp_connection_mainloop(&con_clients, on_client_received, NULL);

    // fermeture de la connection qui a écouté les clients
    tcp_connection_close(&con_clients);

    return NULL;
}


int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage: %s ip_server port_server port_clients\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(port_server < 5000 || port_server > 65000){
        fprintf(stderr, "Bad value of port_server !\n");
        exit(EXIT_FAILURE);
    }
    if(port_clients < 5000 || port_clients > 65000){
        fprintf(stderr, "Bad value of port_clients !\n");
        exit(EXIT_FAILURE);
    }

    
    ip_server = argv[1];
    port_server = atoi(argv[2]);
    port_clients = atoi(argv[3]);

    TCHK( sem_init(&semaphore, 0, 0) );
    etat_server = SERVER_WAIT_CONNECTION;

    pthread_t thread_server;
    pthread_t thread_clients;

    TCHK( pthread_create(&thread_server, NULL, gestion_server, NULL) );

    TCHK( sem_wait(&semaphore) );
    TCHK( sem_destroy(&semaphore) );

    if(etat_server != SERVER_OK){

        TCHK( pthread_join(thread_server, NULL) );

        fprintf(stderr, "Cannot connect to server!\n");
        exit(EXIT_FAILURE);
    }


    TCHK( pthread_create(&thread_clients, NULL, gestion_clients, NULL) );

    //

    //

    TCHK( pthread_join(thread_server, NULL) );

    TCHK( pthread_join(thread_clients, NULL) );
    
    return EXIT_SUCCESS;
}






