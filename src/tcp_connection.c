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

#include "tcp_connection.h"
#include "lib_chks.h"


/**
 * @brief Initialize a message with default values (empty message).
 * @note This function is used to avoid undefined behaviors
 *       when manipulating uninitialized data.
 * 
 * @param msg The message to initialize (passed by reference)
 */
void init_empty_message(Message* msg){
    msg->msg_type = MSG_NULL;
    msg->msg_id = -1;
    msg->proxy_client_socket = -1;
    msg->msg_length = 0;
    msg->dst_flag = 0;
    CHKN( strcpy(msg->dst, "") );
    CHKN( strcpy(msg->src_pseudo, "") );
}


// Initialization of a tcp server socket
void tcp_connection_server_init(TcpConnection* con,
                 char address_receptor[], int port_receptor,
                 int nb_max_connections_server,
                 int timeout_server)
{
    con->type_connection = TCP_CON_SERVER;

    // Create socket
    CHK( con->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0) );

    // The server accepts any address
    con->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Address family
    con->addr.sin_family = AF_INET;

    // Collection of the port of the receptor
    con->addr.sin_port = htons(port_receptor);

    // IPv4 address of the receptor
    inet_aton(address_receptor, &(con->addr.sin_addr));

    // Association of the socket and the network parameters of the receptor
    if (bind(con->sockfd, (SOCKADDR *)&con->addr, sizeof(con->addr)) != 0){
        perror("Error during bind call -> ");
        exit(errno);
    }

    // Preparation to listen to new connections
    printf("Listen\n");
    CHK( listen(con->sockfd, nb_max_connections_server) );

    // Polling initialization
    
    // - Reset
    CHKN( memset(con->poll_fds,    0, sizeof(con->poll_fds)) );
    CHKN( memset(con->poll_addrs,  0, sizeof(con->poll_addrs)) );
    CHKN( memset(con->poll_ad_len, 0, sizeof(con->poll_ad_len)) );

    // - Initial listener socket initialization
    con->poll_fds[0].fd = con->sockfd;
    con->poll_fds[0].events = POLLIN;
    con->nb_poll_fds = 1;

    // - Initialization of the listening of stdin events
    con->poll_fds[1].fd = stdin_fd;
    con->poll_fds[1].events = POLLIN;
    con->nb_poll_fds = 2;

    // Timeout value in milliseconds
    con->timeout = (timeout_server > 0) ? timeout_server * 60 * 1000 : -1;

    con->end_connection = false;
    con->need_compress_poll_arr = false;

    // Init as empty message to avoid undefined behaviors
    init_empty_message(&(con->msg));
}

// Initialization of a tcp client connection
void tcp_connection_client_init(TcpConnection* con,
                                char* ip_to_connect, int port_to_connect,
                                int timeout_client)
{
    con->type_connection = TCP_CON_CLIENT;

    // Create socket
    CHK( con->sockfd = socket(AF_INET, SOCK_STREAM, 0) );

    // Initialize server address
    con->addr.sin_family = AF_INET;
    inet_aton(ip_to_connect, &con->addr.sin_addr);
    con->addr.sin_port = htons(port_to_connect);

    // Connect to server
    CHK( connect(con->sockfd, (SOCKADDR*)&con->addr, sizeof(SOCKADDR)) );

    // Polling initialization
    
    // - Reset
    CHKN( memset(con->poll_fds,    0, sizeof(con->poll_fds)) );
    CHKN( memset(con->poll_addrs,  0, sizeof(con->poll_addrs)) );
    CHKN( memset(con->poll_ad_len, 0, sizeof(con->poll_ad_len)) );

    // - Initial listener socket initialization
    con->poll_fds[0].fd = con->sockfd;
    con->poll_fds[0].events = POLLIN;
    con->nb_poll_fds = 1;

    // - Initialization of the listening of stdin events
    con->poll_fds[1].fd = stdin_fd;
    con->poll_fds[1].events = POLLIN;
    con->nb_poll_fds = 2;

    // Timeout value in milliseconds
    con->timeout = (timeout_client > 0) ? timeout_client * 60 * 1000 : -1;

    con->end_connection = false;
    con->need_compress_poll_arr = false;
}


// Test of potential errors when calling the poll function
bool test_poll_errors(int rc)
{
    // Poll got an error
    if (rc < 0) {
        perror("  poll() failed");
        return true;
    }

    // Poll stayed inactive for a certain time -> Timeout
    if (rc == 0) {
        fprintf(stderr, "  poll() timed out.  End program.\n");
        return true;
    }
    
    // No error
    return false;
}


// Listening to all new connection requests
void new_clients_acceptation(TcpConnection* con) {
    // Variable to stock sockets
    SOCKET new_sock;

    do {

        if (con->nb_poll_fds < MAX_POLL_SOCKETS) {

            SOCKADDR_IN connected_addr;
            socklen_t con_addr_len;

            printf("On accepte.\n");
            // Acceptance of the new connection
            new_sock = accept(con->sockfd, (SOCKADDR*)&connected_addr,
                              &con_addr_len);

            // Error test
            if (new_sock < 0) {

                // Serious error, quitting the server
                if (errno != EWOULDBLOCK) {
                    perror("  accept() failed");
                    con->end_connection = true;
                    break;
                }

                continue;
            }

            printf("  New incoming connection - %d - %d\n",
                        new_sock, connected_addr.sin_addr.s_addr);

            // Registering the new connection
            con->poll_fds[con->nb_poll_fds].fd = new_sock;
            con->poll_fds[con->nb_poll_fds].events = POLLIN;
            con->poll_addrs[con->nb_poll_fds] = connected_addr;
            con->poll_ad_len[con->nb_poll_fds] = con_addr_len;
            con->nb_poll_fds++;
            printf("Nb poll file descriptors : %ld\n", con->nb_poll_fds);
            
        } else {
            printf("  New incoming connection refused:"
                    " maximum connection reached.\n");
        }

    }
    while (new_sock != -1);

    printf("End of acceptance\n");

}


// Compression of the con->poll_fds array
void compress_poll_socket_array(TcpConnection* con, int current_nb_poll_socks)
{
    // last_ok_socket contains the index of the last non-closed socket
    int last_ok_sock = current_nb_poll_socks;            
    while (last_ok_sock > 0 && con->poll_fds[last_ok_sock].fd == -1)
        { last_ok_sock--; }

    // Testing each socket descriptor
    for (int i = 0; i < last_ok_sock; i++) {
        
        // If closed socket, we put the last non-closed socket found
        if (con->poll_fds[i].fd == -1) {
            con->poll_fds[i].fd = con->poll_fds[last_ok_sock].fd;
            con->poll_addrs[i] = con->poll_addrs[last_ok_sock];
            con->poll_ad_len[i] = con->poll_ad_len[last_ok_sock];
            con->poll_fds[last_ok_sock].fd = -1;
            // Updating the last non-closed socket
            while(last_ok_sock > 0 && con->poll_fds[last_ok_sock].fd == -1)
                { last_ok_sock--; }
        }
    }

    // Closure of the server if there are no more active sockets
    if (con->poll_fds[0].fd == -1) {
        con->nb_poll_fds = 0;
        con->end_connection = true;
    } else {
        // Updating the number of still opened sockets
        con->nb_poll_fds = last_ok_sock + 1;
    }
}


// Lit tous les messages possibles sur le socket con->poll_fds[id_poll].fd
//   Pour chaque message lu, appelle la fonction on_msg sur le message reçu
void read_poll_socket(TcpConnection* con, int id_poll,
                      fn_on_msg on_msg, void* on_msg_custom_args)
{

    // Variable pour savoir si un socket du poll se coupe ou pas
    bool close_conn = false;

    do {
        // On lit sur le socket
        // rc pour Return Code
        int rc = recv(con->poll_fds[id_poll].fd,
                      &(con->msg), sizeof(con->msg), MSG_DONTWAIT);

        // Plus rien à lire
        if (rc < 0){
            // Erreur lors de la lecture
            if (errno != EWOULDBLOCK) {
                perror("  recv() failed");
                close_conn = true;
            }
            break;
        }

        // Connection fermée par le client
        if (rc == 0){
            printf("  Connection closed\n");
            close_conn = true;
            break;
        }

        // On a reçu des données
        size_t msg_len = rc;

        printf("Msg reçu : %s\n", con->msg.msg);

        if (con->type_connection == TCP_CON_PROXY_CLIENTS_SIDE){
            con->msg.proxy_client_socket = id_poll;
            printf("Le message de %s a bien recu son id de poll proxy : %d\n",
                                                 con->msg.src_pseudo, id_poll);
        }

        on_msg(con, con->poll_fds[id_poll].fd, &(con->msg), msg_len,
               on_msg_custom_args);

    } while (true);

    if (close_conn){
        if(con->type_connection == TCP_CON_CLIENT ||
           con->type_connection == TCP_CON_PROXY_SERVER_SIDE)
        {
            // Connection fermée, il faut quitter l'application
            con->end_connection = true;
        }
        CHK( close(con->poll_fds[id_poll].fd) );
        con->poll_fds[id_poll].fd = -1;
        con->need_compress_poll_arr = true;
    }
}


// Boucle principale d'une connection tcp
void tcp_connection_mainloop(TcpConnection* con,
                             fn_on_msg on_msg, void* on_msg_custom_args,
                             fn_on_stdin on_stdin, void* on_stdin_custom_args)
{

    if (on_msg == NULL){
        fprintf(stderr, "Error, on_msg is NULL!\n");
        exit(EXIT_FAILURE);
    }

    // Tant que le serveur tourne
    do{

        printf("Waiting for poll\n");

        // On écoute les sockets avec poll
        // rc pour Return Code
        int rc = poll(con->poll_fds, con->nb_poll_fds, con->timeout);

        printf("test, rc=%d\n", rc);
        if (test_poll_errors(rc))
            break;

        // Pour l'instant, on a tant de poll sockets
        int current_nb_poll_socks = con->nb_poll_fds;

        // On parcours tous nos sockets, 
        for (int i = 0; i < current_nb_poll_socks; i++){

            // Socket inactif
            if (con->poll_fds[i].revents == 0){
                continue;
            }

            // Erreur 
            if (con->poll_fds[i].revents != POLLIN){
                fprintf(stderr, "  Error! revents = %d\n",
                                    con->poll_fds[i].revents);
                con->end_connection = true;
                break;
            }
 
            if ((con->type_connection == TCP_CON_SERVER ||
                con->type_connection == TCP_CON_PROXY_CLIENTS_SIDE) &&
                con->poll_fds[i].fd == con->sockfd)
            {

                // Socket qui écoute les connections entrantes 
                new_clients_acceptation(con);

            } else if ((con->type_connection == TCP_CON_CLIENT ||
                        con->type_connection == TCP_CON_PROXY_SERVER_SIDE) &&
                        con->poll_fds[i].fd == con->sockfd){

                // Socket qui écoute les connections entrantes 
                read_poll_socket(con, i, on_msg, on_msg_custom_args);

            } else if (con->poll_fds[i].fd == stdin_fd) {

                // Evenement stdin

                // Read message from standard input
                char buffer[MAX_MSG_LENGTH];

                int bytes_read = read(stdin_fd, buffer, MAX_MSG_LENGTH);
                if (bytes_read == 0) {
                    printf("User closed input\n");
                    break;
                } else if (bytes_read == -1) {
                    perror("read");
                    break;
                }

                // Replace the last \n by \0
                buffer[bytes_read - 1] = '\0';

                if (on_stdin != NULL){
                    on_stdin(con, buffer, bytes_read, on_stdin_custom_args);
                }

                printf("Vous avez écrit: \"%s\"\n", buffer);

            } else {

                printf("Readable socket : %d\n", con->poll_fds[i].fd);

                // SOCKET lisible
                // On reçoit des données tant que possible

                read_poll_socket(con, i, on_msg, on_msg_custom_args);

            }

        }

        // S'il y a besoin de compresser le tableau des sockets du polling
        //  on remet ensemble côtes à côtes tous les sockets 
        //  on n'a pas besoin de toucher aux attributs revent,
        //  ils sont normalement tous à la valeur POLLIN
        if (con->need_compress_poll_arr){

            printf("Compress polls sockets array.\n");
            con->need_compress_poll_arr = false;

            compress_poll_socket_array(con, current_nb_poll_socks);

        }

    } while (con->end_connection == false);

}


// Fermeture d'une connection tcp
void tcp_connection_close(TcpConnection* con){

    // Fermeture du socket qui écoute les connections entrantes
    CHK( close(con->sockfd) );

    // Fermeture de tous les autres sockets ouverts
    for (unsigned long i=0; i<con->nb_poll_fds; i++){
        if (con->poll_fds[i].fd >= 0
            && con->poll_fds[i].fd != con->sockfd
            && con->poll_fds[i].fd != stdin_fd
        ){
            CHK( close(con->poll_fds[i].fd) );
        }
    }
}


// Update the value of a message
void tcp_connection_message_update(Message *msg, char buffer[MAX_MSG_LENGTH],
                                   uint32_t size, int type, int flag,
                                   char pseudo_source[MAX_NAME_LENGTH],
                                   char destination[MAX_NAME_LENGTH])
{
    if (buffer != NULL)
        CHKN( strcpy(msg->msg, buffer) );
    if (size > MAX_MSG_LENGTH)
        size = MAX_MSG_LENGTH;
    if (size != 0)
        msg->msg_length = size;
    msg->msg_type = type;
    msg->dst_flag = flag;
    if (pseudo_source != NULL)
        CHKN( strcpy(msg->src_pseudo, pseudo_source) );
    if (destination != NULL)
        CHKN( strcpy(msg->dst, destination) );
}


// Send a message to the socked sock
void tcp_connection_message_send(TcpConnection* con, SOCKET sock,
                                 Message* msg)
{

    int bytes_sent = send(sock, msg, sizeof(*msg), 0);

    printf("%d bytes sent!\n", bytes_sent);

    if (bytes_sent == -1) {
        perror("send");
        con->end_connection = true;
    }
}


// Fonction qui copie le contenu d'un message depuis src vers dest
void copy_message(Message* dest, Message* src){
    dest->msg_type = src->msg_type;
    dest->msg_id = src->msg_id;
    CHKN( strcpy(dest->src_pseudo, src->src_pseudo) );
    dest->proxy_client_socket = src->proxy_client_socket;
    dest->dst_flag = src->dst_flag;
    CHKN( strcpy(dest->dst, src->dst) );
    dest->msg_length = src->msg_length;
    CHKN( strcpy(dest->msg, src->msg) );
    //
    // TODO: copier les variables qui gèrent les codes correcteurs
}
