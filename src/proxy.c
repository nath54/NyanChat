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

#include "code_errors.h"

#include "lib_chks.h"
#include "useful_lib.h"

/* -------------------- GLOBAL VARIABLES -------------------- */

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

/* ----------------- END OF GLOBAL VARIABLES -----------------*/


void on_client_received(TcpConnection* con, SOCKET sock,
                        Message* msg, size_t msg_length,
                        void* custom_args)
{
    (void)con;
    (void)sock;
    (void)msg_length;
    (void)custom_args;

    if (msg->msg_type == MSG_NULL)
        return;

    // The parent  function is supposed to have put
    // the id of the client poll socket in msg.proxy_client_socket

    if (msg->msg_type == MSG_STD_CLIENT_SERVER && msg->msg_length >= 10)
        // Potential additions of errors
        { code_add_errors_to_msg(msg); }

    tcp_connection_message_send(&con_server, con_server.poll_fds[0].fd, msg);

}


void on_server_received(TcpConnection* con, SOCKET sock,
                        Message* msg, size_t msg_length,
                        void* custom_args)
{
    (void)con;
    (void)sock;
    (void)msg_length;
    (void)custom_args;

    if (msg->msg_type == MSG_NULL)
        return;

    // Test of the client socket to whom to retransmit the message
    if (msg->proxy_client_socket == -1) {
        fprintf(stderr, "Error: Unknown client socket on msg from serv!\n");
        return;
    }
    else if (msg->proxy_client_socket < 0 ||
            (size_t)msg->proxy_client_socket >= con_clients.nb_poll_fds)
    {
        fprintf(stderr,
                "Error: Bad value of client socket on msg  from serv %d / %ld!\n",
                msg->proxy_client_socket, con->nb_poll_fds);
        return;
    }

    // The socket seems to be good, transmission of the message
    tcp_connection_message_send(&con_clients,
                            con_clients.poll_fds[msg->proxy_client_socket].fd,
                            msg);

}


void* gestion_server(void* arg)
{
    (void)arg;

    printf("Before server connection!\n");

    // Initialization of the connection that will listen to the server
    tcp_connection_client_init(&con_server, ip_server, port_server, -1);
    con_server.type_connection = TCP_CON_PROXY_SERVER_SIDE;

    printf("After server connection!\n");

    etat_server = SERVER_OK;
    sem_post(&semaphore);

    printf("Semaphore passed!\n");

    // Main loop of the tcp connection that listens to the server
    tcp_connection_mainloop(&con_server, on_server_received, NULL, NULL, NULL);

    // Closure of the connection that listened to the server
    tcp_connection_close(&con_server);

    // If connection has been cut here, we must cut the clients side
    con_clients.end_connection = true;

    return NULL;
}


void* gestion_clients(void* arg)
{
    (void)arg;

    // Initialization of the connection that will listen to the clients
    tcp_connection_server_init(&con_clients, "127.0.0.1", port_clients, 20, -1);
    con_clients.type_connection = TCP_CON_PROXY_CLIENTS_SIDE;

    // Main loop of the tcp connection that listens to the clients
    tcp_connection_mainloop(&con_clients,
                            on_client_received, NULL,
                            NULL, NULL);

    // Closure of the connection that listened to the clients
    tcp_connection_close(&con_clients);

    return NULL;
}


// TODO: résoudre le problème de nb_poll_fds qui ne s'incrémente pas

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage: %s ip_server port_server port_clients\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ip_server = argv[1];
    port_server = atoi(argv[2]);
    port_clients = atoi(argv[3]);

    if (port_server < 5000 || port_server > 65000) {
        fprintf(stderr, "Bad value of port_server: %d !\n", port_server);
        exit(EXIT_FAILURE);
    }
    if (port_clients < 5000 || port_clients > 65000) {
        fprintf(stderr, "Bad value of port_clients: %d !\n", port_clients);
        exit(EXIT_FAILURE);
    }

    // init random
    srand(time(NULL));

    TCHK( sem_init(&semaphore, 0, 0) );
    etat_server = SERVER_WAIT_CONNECTION;

    pthread_t thread_server;
    pthread_t thread_clients;

    TCHK( pthread_create(&thread_server, NULL, gestion_server, NULL) );

    printf("Waiting for semaphore...\n");

    TCHK( sem_wait(&semaphore) );
    TCHK( sem_destroy(&semaphore) );

    printf("Semaphore passed on the other side!\n");

    if (etat_server != SERVER_OK) {
        TCHK( pthread_join(thread_server, NULL) );
        fprintf(stderr, "Cannot connect to the server!\n");
        exit(EXIT_FAILURE);
    }


    TCHK( pthread_create(&thread_clients, NULL, gestion_clients, NULL) );

    TCHK( pthread_join(thread_server, NULL) );

    TCHK( pthread_join(thread_clients, NULL) );
    
    return EXIT_SUCCESS;
}
