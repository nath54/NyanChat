#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// #include "../include/tcp_connection.h"
#include "../include/lib_ansi.h"
#include "../include/rsa.h"
#include "../include/lib_chks.h"
#include "../include/client.h"

#define PATH_RSA_KEYS "client_rsa_keys/"


// Find the next free slot of the msg_waiting_acq
int find_next_msg_id(ClientState* cstate){
    size_t first_free = 0;
    if(cstate->nb_msg_waiting_acq == cstate->tot_msg_waiting_acq){
        first_free = cstate->tot_msg_waiting_acq;
        cstate->tot_msg_waiting_acq *= 2;
        cstate->msg_waiting_acq = realloc(
                cstate->msg_waiting_acq,
                sizeof(Message)*cstate->tot_msg_waiting_acq
        );
        if(cstate->msg_waiting_acq == NULL){
            fprintf(stderr, "Erreur Realloc!\n");
            exit(EXIT_FAILURE);
        }
        //
        for(size_t i = first_free; i<cstate->tot_msg_waiting_acq; i++){
            cstate->msg_waiting_acq[i].type_msg = MSG_NULL;
        }
    }
    //
    for(size_t i=first_free; i<cstate->tot_msg_waiting_acq; i++){
        if(cstate->msg_waiting_acq[i].type_msg == MSG_NULL){
            return i;
        }
    }
    //
    fprintf(stderr, "Erreur programme, panic!\n");
    exit(EXIT_FAILURE);
}


void on_stdin_client(TcpConnection* con,
                    char msg[T_MSG_MAX],
                    size_t msg_len,
                    void* custom_args)
{
    ClientState* cstate = custom_args;

    if( strlen(cstate->pseudo) == 0 ){
        // Pas de pseudo, pas connecté, donc soit:
        //  - attente de l'entrée utilisateur pour le pseudo
        //  - attente du serveur pour confirmation du pseudo

        if(cstate->waiting_pseudo_confirmation){
            // On ne fait rien, on attend le serveur

        }
        else{
            // msg est censé contenir le pseudo demandé par le client

            // On crée clée RSA puis
            //  on envoie une demande de pseudo au serveur

            if(msg_len < 4){
                printf("Pseudo trop court, "
                       "doit avoir une taille entre 4 et 64 !\n"
                       "\nEntrez votre pseudo > ");
            }
            else{
                // Création du répertoire pour fichiers clés si non existant
                struct stat st = {0};

                if (stat(PATH_RSA_KEYS, &st) == -1) {
                    CHK( mkdir(PATH_RSA_KEYS, 0700) );
                }

                // Répertoire des clés pour le pseudo demandé
                char path_dir[T_NOM_MAX + 100] = PATH_RSA_KEYS;
                CHKN( strcat(path_dir, msg) );
                CHKN( strcat(path_dir, "/") );

                // Chemins des clés
                char path_pub[T_NOM_MAX + 100];
                CHKN( strcpy(path_pub, path_dir) );
                CHKN( strcat(path_pub, "rsa_pub") );
                char path_priv[T_NOM_MAX + 100];
                CHKN( strcpy(path_priv, path_dir) );
                CHKN( strcat(path_priv, "rsa_pub") );

                // Test inscription (première connexion avec ce pseudo)
                if (stat(path_dir, &st) == -1) {
                    // Il faut créer les fichiers clés RSA

                    // Création du répertoire pour le pseudo
                    CHK( mkdir(path_dir, 0700) );

                    // Création des clés
                    if( generate_keypair(path_priv, path_pub,
                                         T_MSG_MAX/2) != 1)
                    {
                        fprintf(stderr, "Erreur génération des clés rsa!\n");
                        exit(EXIT_FAILURE);
                    }
                }

                // Envoi de la demande de connection au serveur
                init_empty_message(&(cstate->msg_waiting_acq[0]));
                cstate->msg_waiting_acq[0].type_msg = MSG_CONNECTION_CLIENT;
                cstate->msg_waiting_acq[0].id_msg = 0;
                strcpy(cstate->msg_waiting_acq[0].pseudo_source, msg);
                load_rsa_key(path_pub,
                             cstate->msg_waiting_acq[0].msg,
                             T_MSG_MAX,
                             &(cstate->msg_waiting_acq[0].taille_msg));

                cstate->nb_msg_waiting_acq += 1;
                tcp_connection_send_struct_message(con, con->sockfd,
                                                &(cstate->msg_waiting_acq[0]));

                cstate->waiting_pseudo_confirmation = true;
            }
        }
    }
    else if( cstate->connected ){
        // Bien connecté, on envoie des messages normalement

        int id_new_msg = find_next_msg_id(cstate);

        cstate->msg_waiting_acq[id_new_msg].type_msg = 1;
        cstate->msg_waiting_acq[id_new_msg].id_msg = id_new_msg;
        strcpy(cstate->msg_waiting_acq[id_new_msg].pseudo_source,
                                                    cstate->pseudo);
        cstate->msg_waiting_acq[id_new_msg].flag_destination = 
                                                    cstate->type_current_dest;
        strcpy(cstate->msg_waiting_acq[id_new_msg].destination,
                                                    cstate->destination);
        cstate->msg_waiting_acq[id_new_msg].proxy_client_socket = MSG_NULL;
        strcpy(cstate->msg_waiting_acq[id_new_msg].msg, msg);
        cstate->msg_waiting_acq[id_new_msg].taille_msg = msg_len;

        cstate->nb_msg_waiting_acq += 1;

        tcp_connection_send_struct_message(con, con->sockfd,
                                    &(cstate->msg_waiting_acq[id_new_msg]));
    }

}


void on_msg_client(TcpConnection* con, SOCKET sock, 
                   Message* msg, size_t msg_len,
                   void* custom_args){
    (void)con;
    (void)sock;
    (void)msg_len;
    ClientState* cstate = custom_args;
    (void)cstate;
    
    printf("Message received: %s\n", msg->msg);

}


// Initialise le client_state
void init_client_state(ClientState* client_state){
    // Init pseudo
    memset(client_state->pseudo, 0, T_NOM_MAX);
    strcpy(client_state->pseudo, "");

    // Init bools
    client_state->connected = false;
    client_state->waiting_pseudo_confirmation = false;

    // Init salon
    client_state->type_current_dest = 0;
    memset(client_state->destination, 0, T_NOM_MAX);
    strcpy(client_state->destination, "");

    // Init msg_waiting_acq
    client_state->tot_msg_waiting_acq = 10;
    client_state->nb_msg_waiting_acq = 0;
    client_state->msg_waiting_acq = calloc(client_state->tot_msg_waiting_acq,
                                           sizeof(Message));
    //
    for(size_t i=0; i<client_state->tot_msg_waiting_acq; i++){
        client_state->msg_waiting_acq[i].type_msg = -1;
    }
    //
    if(client_state->msg_waiting_acq == NULL){
        fprintf(stderr, "Error malloc! \n");
        exit(EXIT_FAILURE);
    }

    // Demande du pseudo client, première chose à faire
    print_rainbow("Bienvenue sur NyanChat!\n");
    printf("\nEntrez votre pseudo > ");
}


void free_client_state(ClientState* client_state){
    free(client_state->msg_waiting_acq);
}


int main(int argc, char* argv[]) {
    
    ClientState client_state;
    TcpConnection con;

    // Check arguments
    if (argc != 3) {
        printf("Usage: %s ip_proxy port_proxy\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* ip_proxy = argv[1];
    int port_proxy = atoi(argv[2]);

    init_client_state(&client_state);
    tcp_connection_client_init(&con, ip_proxy, port_proxy, -1);

    tcp_connection_mainloop(&con,
                            on_msg_client, &client_state,
                            on_stdin_client, &client_state);

    tcp_connection_close(&con);
    free_client_state(&client_state);

    return 0;
}
