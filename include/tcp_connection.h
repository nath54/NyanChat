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

#define T_MAX 1024
#define NB_MAX_CLIENTS 200
#define NB_MAX_MESSAGES 2000

#define MAX_POLL_SOCKETS 200

#define TCP_CONNECTION_SERVER 0
#define TCP_CONNECTION_CLIENT 1


typedef struct {	
    // (0=je suis là, 1=msg normal,
    //  2=demande qui est là, 3=au revoir, 4=erreur)
	int 	type_msg;
    
    // ip du client
	uint32 	ip_source;
    
    // 0 = msg privé, 1 = salon privé, 2 = salon par défaut
	int 	flag_destination;
    
    // ip du client destinataire, ou alors id du salon
	uint32 	destination;
    
    // Taille du message
	uint32 	taille_msg;
    
    // Message de l’utilisateur
	char msg[T_MAX];
    
    // (pour détection & correction)
	// ???	code;
} Message;

// Forward declaration
struct TcpConnection;

typedef struct TcpConnection{
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
    int type_connection;    // 0 = server, 1 = client
    int timeout;    // Temps max d'inactivité avant fermeture de la connexion
    bool end_connection; // S'il faut éteindre la connexion
    bool need_compress_poll_arr; // S'il faut compresser `this->poll_fds`
} TcpConnection;

typedef void(fn_on_msg)(TcpConnection* con, SOCKET sock,
                        Message msg, size_t msg_len);

typedef void(fn_on_stdin)(TcpConnection* con,
                          char msg[T_MAX], size_t msg_len);


static socklen_t sockaddr_size = sizeof(SOCKADDR_IN);

// static int stdin_fd = fileno(stdin);
#define stdin_fd fileno(stdin)

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
                                    fn_on_msg on_msg,
                                    fn_on_stdin on_stdin);



// Fermeture d'une connection tcp
void tcp_connection_close(TcpConnection* con);


// Envoi d'un message
void tcp_send_message(TcpConnection* con, SOCKET sock,
                      char buffer[T_MAX], int message_size, int flags,
                      uint32 ip_src, uint32 ip_dest);