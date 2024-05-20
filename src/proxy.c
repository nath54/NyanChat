#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <math.h>
#include <sys/random.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include "tcp_connection.h"

#include "codes_detection_correction.h"

#include "lib_chks.h"

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


int randint(int maxi_val){
    return rand() % maxi_val;
}


void on_client_received(TcpConnection* con, SOCKET sock,
                        Message* msg, size_t msg_length,
                        void* custom_args)
{
    (void)con;
    (void)sock;
    (void)msg_length;
    (void)custom_args;

    if(msg->type_msg == MSG_NULL)
        return;

    //

    // Normalement, la fonction qui a appelé cette fonction 
    //  a mis l'id du poll socket du client dans msg.proxy_client_socket

    if(msg->type_msg == MSG_NORMAL_CLIENT_SERVER && msg->taille_msg >= 10){
        // Ajouts potentiel d'erreurs

        if(randint(100) <= PROXY_ERROR_RATE){

            // Ajout d'erreurs au message
            code_add_noise_to_msg(msg, randint(PROXY_MAX_ERROR_CREATED));
        }

    }

    tcp_connection_send_struct_message(&con_server, con_server.poll_fds[0].fd,
                                       msg);

}


void on_server_received(TcpConnection* con, SOCKET sock,
                        Message* msg, size_t msg_length,
                        void* custom_args)
{
    (void)con;
    (void)sock;
    (void)msg_length;
    (void)custom_args;

    if(msg->type_msg == MSG_NULL)
        return;

    // Test du socket client à qui retransmettre le message
    if(msg->proxy_client_socket == -1){
        fprintf(stderr, "Error: Unknown client socket on msg from serv!\n");
        return;
    }
    else if(msg->proxy_client_socket < 0||
            (size_t)msg->proxy_client_socket >= con->nb_poll_fds)
    {
        fprintf(stderr,
                "Error: Bad value of client socket on msg  from serv!\n");
        return;
    }

    // Le socket est à priori bon, on transmet le message
    tcp_connection_send_struct_message(
        &con_clients,
        con_clients.poll_fds[msg->proxy_client_socket].fd,
        msg
    );

}


void* gestion_server(void* arg)
{
    (void)arg;
    // Initialisation de la connection qui va écouter le serveur
    tcp_connection_client_init(&con_server, ip_server, port_server, -1);
    con_server.type_connection = TCP_CON_PROXY_SERVER_SIDE;

    // Boucle principale de la connection tcp qui écoute le serveur
    tcp_connection_mainloop(&con_server,
                            on_server_received, NULL,
                            NULL, NULL);

    // fermeture de la connection qui a écouté le serveur
    tcp_connection_close(&con_server);

    return NULL;
}


void* gestion_clients(void* arg)
{
    (void)arg;
    // Initialisation de la connection qui va écouter les clients
    tcp_connection_server_init(&con_clients,
                               "127.0.0.1", port_clients, 20, -1);
    con_clients.type_connection = TCP_CON_PROXY_CLIENTS_SIDE;


    // Boucle principale de la connection tcp qui écoute les clients
    tcp_connection_mainloop(&con_clients,
                            on_client_received, NULL,
                            NULL, NULL);

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

    // init random
    srand(time(NULL));

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






