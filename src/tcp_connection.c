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


// Initialisation d'un message vide
//  pour éviter de manipuler des données non initialisées
void init_empty_message(Message* msg){
    msg->msg_type = MSG_NULL;
    msg->msg_id = -1;
    msg->proxy_client_socket = -1;
    msg->msg_length = 0;
    msg->dst_flag = 0;
    strcpy(msg->dst, "");
    strcpy(msg->src_pseudo, "");
}


// Initialisation d'un socket pour un serveur tcp
void tcp_connection_server_init(TcpConnection* con,
                 char address_receptor[], int port_receptor,
                 int nb_max_connections_server,
                 int timeout_server)
{

    con->type_connection = TCP_CON_SERVER;

    // Création du socket
    con->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    // Test de bonne création
    if (con->sockfd == INVALID_SOCKET){
        fprintf(stderr, "Erreur lors de la création du socket!\n");
        exit(errno);
    }

    // nous sommes un serveur, nous acceptons n'importe quelle adresse
    con->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // famille d'adresse
    con->addr.sin_family = AF_INET;

    // recuperation du port du recepteur
    con->addr.sin_port = htons(port_receptor);

    // adresse IPv4 du recepteur
    inet_aton(address_receptor, &(con->addr.sin_addr));

    // association de la socket et des param reseaux du recepteur
    if (bind(con->sockfd, (SOCKADDR *)&con->addr, sizeof(con->addr)) != 0){
        perror("erreur lors de l'appel a bind -> ");
        exit(errno);
    }

    // Préparation à l'écoute des nouvelles connections
    if (listen(con->sockfd, nb_max_connections_server) == SOCKET_ERROR) {
        perror("listen()");
        exit(errno);
    }

    // Initialisation du polling
    
    // - Mise à zéro 
    memset(con->poll_fds, 0 , sizeof(con->poll_fds));
    memset(con->poll_addrs, 0 , sizeof(con->poll_addrs));
    memset(con->poll_ad_len, 0 , sizeof(con->poll_ad_len));

    // - Initialisaiton du socket écouteur initial
    con->poll_fds[0].fd = con->sockfd;
    con->poll_fds[0].events = POLLIN;
    con->nb_poll_fds = 1;

    // - Initialisation de l'écoute des évenements stdin
    con->poll_fds[1].fd = stdin_fd;
    con->poll_fds[1].events = POLLIN;
    con->nb_poll_fds = 2;

    // Valeur du timeout en milisecondes
    if (timeout_server > 0){
        con->timeout = timeout_server * 60 * 1000;
    }
    else {
        con->timeout = -1;
    }

    con->end_connection = false;
    con->need_compress_poll_arr = false;

    // Init message vide, pour éviter undefined behaviors
    init_empty_message(&(con->msg));
}

void tcp_connection_client_init(TcpConnection* con,
                                char* ip_to_connect, int port_to_connect,
                                int timeout_client){

    con->type_connection = TCP_CON_CLIENT;

    // Create socket
    con->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (con->sockfd == INVALID_SOCKET) {
        perror("socket");
        exit(errno);
    }

    // Initialize server address
    con->addr.sin_family = AF_INET;
    inet_aton(ip_to_connect, &con->addr.sin_addr);
    con->addr.sin_port = htons(port_to_connect);

    // Connect to server
    if (connect(con->sockfd,
                    (SOCKADDR*)&con->addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect");
        exit(errno);
    }

    
    // Initialisation du polling
    
    // - Mise à zéro 
    memset(con->poll_fds, 0 , sizeof(con->poll_fds));
    memset(con->poll_addrs, 0 , sizeof(con->poll_addrs));
    memset(con->poll_ad_len, 0 , sizeof(con->poll_ad_len));

    // - Initialisaiton du socket écouteur initial
    con->poll_fds[0].fd = con->sockfd;
    con->poll_fds[0].events = POLLIN;
    con->nb_poll_fds = 1;

    // - Initialisation de l'écoute des évenements stdin
    con->poll_fds[1].fd = stdin_fd;
    con->poll_fds[1].events = POLLIN;
    con->nb_poll_fds = 2;

    // Valeur du timeout en milisecondes
    if (timeout_client > 0){
        con->timeout = timeout_client * 60 * 1000;
    }
    else {
        con->timeout = -1;
    }

    con->end_connection = false;
    con->need_compress_poll_arr = false;
}




// Test des erreurs potentielles lors de l'appel à la fonction poll
bool test_poll_errors(int rc){
    // Poll a eu une erreur
    if(rc < 0){
        perror("  poll() failed");
        return true;
    }

    // Poll est resté inactif pendant un certain temps -> Timeout
    if(rc == 0){
        fprintf(stderr, "  poll() timed out.  End program.\n");
        return true;
    }
    
    // Il n'y a pas eu d'erreurs
    return false;
}

// On écoute toutes les nouvelles demandes de connections
void new_clients_acceptation(TcpConnection* con) {
    // Variable pour stoquer des sockets
    SOCKET new_sock;

    do {

        if (con->nb_poll_fds < MAX_POLL_SOCKETS){

            SOCKADDR_IN connected_addr;
            socklen_t con_addr_len;

            printf("On accepte.\n");
            // Acceptation de la nouvelle connection
            new_sock = accept(con->sockfd,
                                (SOCKADDR*)&connected_addr, &con_addr_len);

            // Test erreur
            if(new_sock < 0){

                // Erreur grave, on quitte le serveur
                if (errno != EWOULDBLOCK){
                    perror("  accept() failed");
                    con->end_connection = true;
                    break;
                }
            }

            printf("  New incoming connection - %d - %d\n",
                        new_sock, connected_addr.sin_addr.s_addr);

            // Enregistrement de la nouvelle connection
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
    while(new_sock != -1);

    printf("Fin acceptation\n");

}


void compress_poll_socket_array(TcpConnection* con, int current_nb_poll_socks){

    // last_ok_sock contient l'index du dernier socket non fermé
    int last_ok_sock = current_nb_poll_socks;            
    while (last_ok_sock > 0 && con->poll_fds[last_ok_sock].fd == -1){
        last_ok_sock--;
    }

    // On teste chaque descripteur de socket
    for (int i=0; i < last_ok_sock; i++){
        
        // Si socket fermé
        //   -> on met à la place le dernier socket non fermé trouvé
        if(con->poll_fds[i].fd == -1){
            con->poll_fds[i].fd = con->poll_fds[last_ok_sock].fd;
            con->poll_addrs[i] = con->poll_addrs[last_ok_sock];
            con->poll_ad_len[i] = con->poll_ad_len[last_ok_sock];
            con->poll_fds[last_ok_sock].fd = -1;
            // On remet à jour le dernier socket non fermé
            while(last_ok_sock > 0
                    && con->poll_fds[last_ok_sock].fd == -1)
            {
                last_ok_sock--;
            }
        }
    }

    // S'il n'y a plus de sockets actifs, on ferme le serveur
    if(con->poll_fds[0].fd == -1){
        con->nb_poll_fds = 0;
        con->end_connection = true;
    }
    else{
        // On met à jour le nombre de sockets toujours ouverts
        con->nb_poll_fds = last_ok_sock + 1;
    }
}


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
        }

        on_msg(con, con->poll_fds[id_poll].fd, &(con->msg), msg_len,
               on_msg_custom_args);

    } while (true);

    if (close_conn){
        close(con->poll_fds[id_poll].fd);
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
    close(con->sockfd);

    // Fermeture de tous les autres sockets ouverts
    for (unsigned long i=0; i<con->nb_poll_fds; i++){
        if (con->poll_fds[i].fd >= 0
            && con->poll_fds[i].fd != con->sockfd
            && con->poll_fds[i].fd != stdin_fd
        ){
            close(con->poll_fds[i].fd);
        }
    }
}


void tcp_connection_message_update(Message *msg, char buffer[MAX_MSG_LENGTH],
                                   uint32_t size, int type, int flag,
                                   char pseudo_source[MAX_NAME_LENGTH],
                                   char destination[MAX_NAME_LENGTH])
{
    if (buffer != NULL)
        strcpy(msg->msg, buffer);
    if (size > MAX_MSG_LENGTH)
        size = MAX_MSG_LENGTH;
    if (size != 0)
        msg->msg_length = size;
    msg->msg_type = type;
    msg->dst_flag = flag;
    if (pseudo_source != NULL)
        strcpy(msg->src_pseudo, pseudo_source);
    if (destination != NULL)
        strcpy(msg->dst, destination);
}


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
    strcpy(dest->src_pseudo, src->src_pseudo);
    dest->proxy_client_socket = src->proxy_client_socket;
    dest->dst_flag = src->dst_flag;
    strcpy(dest->dst, src->dst);
    dest->msg_length = src->msg_length;
    strcpy(dest->msg, src->msg);
    //
    // TODO: copier les variables qui gèrent les codes correcteurs
}
