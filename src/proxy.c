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



/* ------------------ VARIABLES GLOBALES ------------------ */

sem_t semaphore;
int etat_server;

#define SERVER_OK 1
#define SERVER_WAIT_CONNECTION 0
#define SERVER_ERROR -1


/* ------------------ FIN VARIABLES GLOBALES ------------------*/



void on_client_received(TcpConnection* con, SOCKET sock,
                        Message msg, size_t msg_length)
{

}



void on_server_received(TcpConnection* con, SOCKET sock,
                        Message msg, size_t msg_length)
{

}


void gestion_server(){

}

void gestion_clients(){

}


int main(int argc, char* argv[]){

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
        exit(1);
    }


    TCHK( pthread_create(&thread_clients, NULL, gestion_clients, NULL) );

    //

    //

    TCHK( pthread_join(thread_server, NULL) );

    TCHK( pthread_join(thread_clients, NULL) );
    
    return EXIT_SUCCESS;
}






