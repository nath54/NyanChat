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
#define MAX_NAME_LENGTH 64
#define MIN_NAME_LENGTH 4

//
#define MAX_MSG_LENGTH 512
#define CODE_LENGTH 256

#define MAX_POLL_SOCKETS 200

#define TCP_CON_SERVER 0
#define TCP_CON_CLIENT 1
#define TCP_CON_PROXY_SERVER_SIDE 2
#define TCP_CON_PROXY_CLIENTS_SIDE 3

#define MSG_NULL -1
#define MSG_STD_CLIENT_SERVER 0
#define MSG_SERVER_CLIENT 1
#define MSG_CONNECTION_CLIENT 2 // Sent when a client try to connect, contain pseudo & connection code
#define MSG_ERROR 4
#define MSG_ACK_POS 5
#define MSG_ACK_NEG 6
#define MSG_WELL_CONNECTED 8  // Indicate a successful connection
#define MSG_CLIENT_CONNECTED 9  // Sent when another connects, contain the pseudo of that client
#define MSG_CLIENT_DISCONNECTED 10  // Sent when another desconnects, contain the pseudo of that client
#define MSG_CLIENT_DISCONNECT 11  // Sent when a client disconnects and quit the app 

#define ERROR_MSG_CON_DIFF_KEYS "Error: this pseudo already has another connection code"
#define ERROR_MSG_BAD_DECODED "Error: misdecoded the connection code"
#define ERROR_MSG_STILL_ACTIVE_CLIENT "Error: you still have an active session"
#define ERROR_ALREADY_MAX_CLIENTS "Error: already max clients connected!"

#define MSG_FLAG_DEFAULT_CHANNEL 0
#define MSG_FLAG_PRIVATE_CHANNEL 1
#define MSG_FLAG_PRIVATE_MESSAGE 2

/* Structure used for messages, errors, acknowledgments,
    and connection information.
    Transmitted by the connection
*/
typedef struct __attribute__((packed, aligned(4))) Message {	
    /*
    Type of message: see MSG_.....
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
    0 = default channel
    1 = private channel
    2 = private message
    */
	int dst_flag;
    
    // Pseudo of the client destination, or name of the channel
	char dst[MAX_NAME_LENGTH];
    
    // Length of the message
	uint32 msg_length;
    
    // Message from the user
	char msg[MAX_MSG_LENGTH];
    
    // (for detection & correction)
    char control[MAX_MSG_LENGTH];

    // error[i] is true if msg[i] has an error not corrected
    bool error[MAX_MSG_LENGTH];
    // Number of retransmissions due to NEGATIVE ACK
    int nb_retransmission;

} Message;


typedef struct TcpConnection {
    // Base TCP Socket Connection
    SOCKET sockfd;              // File descriptor of socket
    SOCKADDR_IN addr;           // @ client (server), @ serveur (client)

    // Polling
    struct pollfd poll_fds[MAX_POLL_SOCKETS]; // Sockets array of polling
    SOCKADDR_IN poll_addrs[MAX_POLL_SOCKETS]; // Addresses of these sockets.
    socklen_t poll_ad_len[MAX_POLL_SOCKETS];  // Taille véritable de ces addrs
    nfds_t nb_poll_fds;    // Current number of sockets in polling.

    // Server vars
    struct Message msg;    // Will receive messages from rcv

    // Connection parameters
    int type_connection;    /* 0 = server, 1 = client,
                               2 = proxy server side, 3 = proxy clients side
                            */

    //
    bool enable_debug_print;
    bool ansi_stdin;

    int timeout;    // Maximum inactivity time before a closure of the connection.
    bool end_connection; // Should the connection be closed?
    bool need_compress_poll_arr; // Should this->poll_fds be compressed?
} TcpConnection;


// Types for custom functions called on events
typedef void(fn_on_msg)(TcpConnection* con, SOCKET sock,
                        Message* msg, size_t msg_len,
                        void* custom_args);

typedef void(fn_on_stdin)(TcpConnection* con,
                          char msg[MAX_MSG_LENGTH], size_t msg_len,
                          void* custom_args);


// static socklen_t sockaddr_size = sizeof(SOCKADDR_IN);

// static int stdin_fd = fileno(stdin);
#define stdin_fd fileno(stdin)

/**
 * @brief Initialiation of an empty message.
 * @note Usefull to avoid manipulating uninitialized data.
 * 
 * @param msg The message to initialize.
 */
void init_empty_message(Message* msg);

/**
 * @brief Initialiation of a socket for a TCP server.
 * 
 * @param con 
 * @param address_receptor 
 * @param port_receptor 
 * @param nb_max_connections_server 
 * @param timeout_server 
 */
void tcp_connection_server_init(TcpConnection* con,
                                char address_receptor[], int port_receptor,
                                int nb_max_connections_server,
                                int timeout_server);

void tcp_connection_client_init(TcpConnection* con,
                                char* ip_to_connect, int port_to_connect,
                                int timeout_client);

/**
 * @brief Main loop of a TCP connection.
 * 
 * @param con 
 * @param on_msg 
 * @param on_msg_custom_args 
 * @param on_stdin 
 * @param on_stdin_custom_args 
 */
void tcp_connection_mainloop(TcpConnection* con,
                             fn_on_msg on_msg, void* on_msg_custom_args,
                             fn_on_stdin on_stdin, void* on_stdin_custom_args);



/**
 * @brief Closure of a TCP connection.
 * 
 * @param con The TCP connection to close.
 */
void tcp_connection_close(TcpConnection* con);


/**
 * @brief Update arguments of a message.
 * 
 * @param msg The message to update.
 * @param buffer The new text of the message.
 * @param size The size of the new text.
 * @param type The new type of the message.
 * @param flag The new flag for the message destination.
 * @param pseudo_source The pseudo of the new source.
 * @param destination The new destination.
 */
void tcp_connection_message_update(Message *msg, char buffer[MAX_MSG_LENGTH],
                                   uint32_t size, int type, int flag,
                                   char pseudo_source[MAX_NAME_LENGTH],
                                   char destination[MAX_NAME_LENGTH]);

/**
 * @brief Transmission of a message.
 * 
 * @param con The connection to close in case of error.
 * @param sock The socket to use.
 * @param msg The message to send.
 */
void tcp_connection_message_send(TcpConnection* con, SOCKET sock,
                                 Message* msg);


/**
 * @brief Copy a message from @p src to @p dest.
 * 
 * @param dest The destination message.
 * @param src The source message.
 */
void copy_message(Message* dest, Message* src);