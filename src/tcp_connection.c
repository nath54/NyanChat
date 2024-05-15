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

#include "../include/tcp_connection.h"



// Initialisation d'un socket pour un serveur tcp
void tcp_connection_init(TcpConnection* con,
                 char address_receptor[], int port_receptor,
                 int nb_max_connections_server,
                 int timeout_server)
{

    // Création du socket
    con->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Test de bonne création
    if(con->sockfd == INVALID_SOCKET){
        fprintf(stderr, "Erreur lors de la création du socket!\n");
        exit(errno);
    }

    // nous sommes un serveur, nous acceptons n'importe quelle adresse
    con->addr_recep.sin_addr.s_addr = htonl(INADDR_ANY);

    // famille d'adresse
    con->addr_recep.sin_family = AF_INET;

    // recuperation du port du recepteur
    con->addr_recep.sin_port = htons(port_receptor);

    // adresse IPv4 du recepteur
    inet_aton(address_receptor, &(con->addr_recep.sin_addr));

    // association de la socket et des param reseaux du recepteur
    if(bind(con->sockfd, (SOCKADDR *)&con->addr_recep, sockaddr_size) != 0){
        perror("erreur lors de l'appel a bind -> ");
        exit(errno);
    }

    // Préparation à l'écoute des nouvelles connections
    if(listen(con->sockfd, nb_max_connections_server) == SOCKET_ERROR) {
        perror("listen()");
        exit(errno);
    }

    // Initialisation du polling
    
    // - Mise à zéro 
    memset(con->poll_fds, 0 , sizeof(con->poll_fds));

    // - Initialisaiton du socket écouteur initial
    con->poll_fds[0].fd = con->sockfd;
    con->poll_fds[0].events = POLLIN;
    con->nb_poll_fds = 1;

    // Valeur du timeout en milisecondes
    con->timeout = timeout_server * 60 * 1000;

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
void new_clients_acceptation(TcpConnection* con, bool* end_server){
    // Variable pour stoquer des sockets
    SOCKET new_sock;

    // Tant que l'on a de nouveaux sockets
    do{

        if(con->nb_poll_fds < MAX_POLL_SOCKETS){

            // Acceptation de la nouvelle connection
            new_sock = accept(con->sockfd, NULL, NULL);

            // Test erreur
            if(new_sock < 0){

                // Erreur grave, on quitte le serveur
                if (errno != EWOULDBLOCK){
                    perror("  accept() failed");
                    *end_server = true;
                }
                break;
            }

            printf("  New incoming connection - %d\n", new_sock);

            // Enregistrement de la nouvelle connection
            con->poll_fds[con->nb_poll_fds].fd = new_sock;
            con->poll_fds[con->nb_poll_fds].events = POLLIN;
            con->nb_poll_fds++;
        } else {
            printf("  New incoming connection refused:"
                    " maximum connection reached.\n");
        }

    } while(new_sock != -1);
}


// Boucle principale d'une connection tcp
void tcp_connection_mainloop(TcpConnection* con, fn_on_msg on_msg){

    // Pour savoir quand le serveur est fini (à cause d'une erreur par exemple)
    bool end_server;

    // rc pour Return Code, sert pour tester le retour de différentes fonctions
    int rc;

    // Variable pour savoir si un socket du poll se coupe ou pas
    bool close_conn;

    // Buffer, pour récupérer les messages qui ont étés envoyés à ce serveur
    char buffer[TAILLE_BUF];

    // Besoin de compresser le tableau con->poll_fds
    bool need_compress_poll_fds = false;

    // Tant que le serveur tourne
    do{

        // On écoute les sockets avec poll
        rc = poll(con->poll_fds, con->nb_poll_fds, con->timeout);
        if(test_poll_errors(rc))
            break;

        // Pour l'instant, on a tant de poll sockets
        int current_nb_poll_socks = con->nb_poll_fds;

        // On parcours tous nos sockets, 
        for(int i = 0; i < current_nb_poll_socks; i++){

            // Socket inactif
            if(con->poll_fds[i].revents == 0){
                continue;
            }

            // Erreur 
            if(con->poll_fds[i].revents != POLLIN){
                fprintf(stderr, "  Error! revents = %d\n",
                                    con->poll_fds[i].revents);
                end_server = true;
                break;
            }
            

            if(con->poll_fds[i].fd == con->sockfd){
                
                // Socket qui écoute les connections entrantes 
                new_clients_acceptation(con, &end_server);

            } else {

                // SOCKET lisible
                // On reçoit des données tant que possible

                close_conn = false;

                do{
                    rc = recv(con->poll_fds[i].fd, buffer, sizeof(buffer), 0);

                    // 
                    if(rc < 0){
                        if (errno != EWOULDBLOCK) {
                            perror("  recv() failed");
                            close_conn = true;
                        }
                        break;
                    }

                    // Connection fermée par le client
                    if(rc == 0){
                        printf("  Connection closed\n");
                        close_conn = true;
                        break;
                    }

                    // On a reçu des données
                    size_t msg_len = rc;

                    on_msg(con, con->poll_fds[i].fd, buffer, msg_len);
                    
                } while(true);

                if(close_conn){
                    close(con->poll_fds[i].fd);
                    con->poll_fds[i].fd = -1;
                    need_compress_poll_fds = true;
                }

            }

        }


        if(need_compress_poll_fds){

            need_compress_poll_fds = false;

            // last_ok_sock contient l'index du dernier socket non fermé
            int last_ok_sock = current_nb_poll_socks;            
            while(last_ok_sock > 0 && con->poll_fds[last_ok_sock].fd == -1){
                last_ok_sock--;
            }

            // On teste chaque descripteur de socket
            for(int i=0; i < last_ok_sock; i++){
                
                // Si socket fermé
                //   -> on met à la place le dernier socket non fermé trouvé
                if(con->poll_fds[i].fd == -1){
                    con->poll_fds[i].fd = con->poll_fds[last_ok_sock].fd;
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
                end_server = true;
            }
            else{
                // On met à jour le nombre de sockets toujours ouverts
                con->nb_poll_fds = last_ok_sock + 1;
            }
        }

    } while(end_server == false);

}



// Fermeture d'une connection tcp
void tcp_connection_close(TcpConnection* con){

    // Fermeture du socket qui écoute les connections entrantes
    close(con->sockfd);

    // Fermeture de tous les autres sockets ouverts
    for(int i=0; i<con->nb_poll_fds; i++){
        if(con->poll_fds[i].fd >= 0 && con->poll_fds[i].fd != con->sockfd){
            close(con->poll_fds[i].fd);
        }
    }
}
