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
#define T_NOM_MAX 256

#define T_MSG_MAX 256

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
#define MSG_ACQ_POS 5
#define MSG_ACQ_NEG 6

// Structure utilisée pour les messages et
//  les acquittements transmis par la connection
typedef struct {	
    /*
    Modifier les defines juste au dessus si changements de valeurs
        (
            -1=msg NULL,
            0=msg normal client->serveur,
            1=msg serveur->client,
            2=connection client
            3=fin de connection,
            4=erreur,
            5=acquittement positif,
            6=acquittement négatif
        ) */
	int type_msg;

    // Identifiant qui permet au client de savoir de quel message
    //  le serveur parle lorsqu'un acquittement est renvoyé
    int id_msg;
    
    // pseudo du client
	char pseudo_source[T_NOM_MAX];

    // Socket client du proxy (rempli par le proxy)
    int proxy_client_socket;
    
    // 0 = msg privé, 1 = salon privé, 2 = salon par défaut
	int flag_destination;
    
    // pseudo du client destinataire, ou alors nom du salon
	char destination[T_NOM_MAX];
    
    // Taille du message
	uint32 taille_msg;
    
    // Message de l’utilisateur
	char msg[T_MSG_MAX];
    
    // (pour détection & correction)
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
    Message msg[1];

    // Paramètres de la connexion
    int type_connection;    // 0 = server, 1 = client,
                            // 2 = proxy côté server, 3 = proxy côté clients
    int timeout;    // Temps max d'inactivité avant fermeture de la connexion
    bool end_connection; // S'il faut éteindre la connexion
    bool need_compress_poll_arr; // S'il faut compresser this->poll_fds
} TcpConnection;



typedef void(fn_on_msg)(TcpConnection* con, SOCKET sock,
                        Message msg, size_t msg_len,
                        void* custom_args);

typedef void(fn_on_stdin)(TcpConnection* con,
                          char msg[T_MSG_MAX], size_t msg_len,
                          void* custom_args);


// static socklen_t sockaddr_size = sizeof(SOCKADDR_IN);

// static int stdin_fd = fileno(stdin);
#define stdin_fd fileno(stdin)


// Initialisation d'un message vide
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


// Envoi d'un message
void tcp_connection_send_message(TcpConnection* con, SOCKET sock,
                                 char buffer[T_MSG_MAX], int message_size,
                                 int flags,
                                 char pseudo_src[T_NOM_MAX],
                                 int type_destination,
                                 char destination[T_NOM_MAX]);


// Transmission d'un struct message
void tcp_connection_send_struct_message(TcpConnection* con, SOCKET sock,
                                        Message msg);
