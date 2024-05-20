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
#include <stdint.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

typedef u_int32_t uint32;

// #define BUFFER_SIZE 1024


// Taille max d'un pseudo de clients ou d'un nom de salon
#define MAX_NAME_LENGTH 256

#define MAX_MSG_LENGTH 256

#define MAX_POLL_SOCKETS 200

#define TCP_CON_SERVER 0
#define TCP_CON_CLIENT 1
#define TCP_CON_PROXY_SERVER_SIDE 2
#define TCP_CON_PROXY_CLIENTS_SIDE 3


#define MSG_NULL -1
#define MSG_NORMAL_CLIENT_SERVER 0
#define MSG_SERVER_CLIENT 1
#define MSG_CONNECTION_CLIENT 2
#define MSG_ERREUR 4
#define MSG_ACK_POS 5
#define MSG_ACK_NEG 6

/* Structure used for messages and acknowledgments
    transmitted by the connection
*/
typedef struct __attribute__((packed, aligned(4))) Message {	
    /*
    Type of message:
        -1 = msg NULL,
        0 = normal msg client->server,
        1 = msg server->client,
        2 = client connection
        3 = end of connection,
        4 = error,
        5 = positive acknowledgment
        6 = negative acknowledgment
    */
	int msg_type;

    /*
    Identifier which allows the client to know which message
     the server is talking about when an acknowledgment is sent
    */
    int msg_id;
    
    // Pseudo of the client
	char src_pseudo[MAX_NAME_LENGTH];

    // Client socket of the proxy (filled by the proxy)
    int proxy_client_socket;
    
    /*
    Flag for destination type:
    0 = private message
    1 = private channel
    2 = default channel
    */
	int dst_flag;
    
    // Pseudo of the client destination, or name of the channel
	char dst[MAX_NAME_LENGTH];
    
    // Length of the message
	uint32 msg_length;
    
    // Message from the user
	char msg[MAX_MSG_LENGTH];
    
    // (for detection & correction)
    // TODO: ajouter ici les ressources nécessaires pour les codes polynomiaux
	// ???	code;

} Message;


typedef struct {
    // Base TCP Socket Connection
    SOCKET sockfd;              // Descripteur du fichier du socket
    SOCKADDR_IN addr;           // @ recepteur (serveur), @ serveur (client)

    // Polling
    struct pollfd poll_fds[MAX_POLL_SOCKETS]; // Tableau des sockets du polling
    SOCKADDR_IN poll_addrs[MAX_POLL_SOCKETS]; // Adresses de ces sockets
    socklen_t poll_ad_len[MAX_POLL_SOCKETS];  // Taille véritable de ces addrs
    nfds_t nb_poll_fds;    // Nombre actuel de sockets de polling

    // Variables pour le serveur
    struct Message msg;    // Recevra les messages depuis recv

    // Paramètres de la connexion
    int type_connection;    // 0 = server, 1 = client,
                            // 2 = proxy côté server, 3 = proxy côté clients
    int timeout;    // Temps max d'inactivité avant fermeture de la connexion
    bool end_connection; // S'il faut éteindre la connexion
    bool need_compress_poll_arr; // S'il faut compresser this->poll_fds
} TcpConnection;



typedef void(fn_on_msg)(TcpConnection* con, SOCKET sock,
                        Message* msg, size_t msg_len,
                        void* custom_args);

typedef void(fn_on_stdin)(TcpConnection* con,
                          char msg[MAX_MSG_LENGTH], size_t msg_len,
                          void* custom_args);


// static socklen_t sockaddr_size = sizeof(SOCKADDR_IN);

// static int stdin_fd = fileno(stdin);
#define stdin_fd fileno(stdin)


// Initialisation d'un message vide
//  pour éviter de manipuler des données non initialisées
void init_empty_message(Message* msg);


// Initialisation d'un socket pour un serveur tcp
void tcp_connection_server_init(TcpConnection* con,
                                char address_receptor[], int port_receptor,
                                int nb_max_connections_server,
                                int timeout_server);

void tcp_connection_client_init(TcpConnection* con,
                                char* ip_to_connect, int port_to_connect,
                                int timeout_client);

// Boucle principale d'une connection tcp
void tcp_connection_mainloop(TcpConnection* con,
                             fn_on_msg on_msg, void* on_msg_custom_args,
                             fn_on_stdin on_stdin, void* on_stdin_custom_args);



// Fermeture d'une connection tcp
void tcp_connection_close(TcpConnection* con);


// Mise à jour d'un message
void tcp_connection_message_update(Message *msg, char buffer[MAX_MSG_LENGTH],
                                   uint32_t size, int type, int flag,
                                   char pseudo_source[MAX_NAME_LENGTH],
                                   char destination[MAX_NAME_LENGTH]);


// Transmission d'un struct message
void tcp_connection_message_send(TcpConnection* con, SOCKET sock,
                                 Message* msg);


// Fonction qui copie le contenu d'un message depuis src vers dest
void copy_message(Message* dest, Message* src);