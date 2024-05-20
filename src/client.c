#include <sys/types.h>
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
#include "../include/client.h"


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
            cstate->msg_waiting_acq[i].type_msg = -1;
        }
    }
    //
    for(size_t i=first_free; i<cstate->tot_msg_waiting_acq; i++){
        if(cstate->msg_waiting_acq[i].type_msg == -1){
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

        if(cstate->attente_confirmation_pseudo){
            // On ne fait rien, on attend le serveur

        }
        else{
            // On crée clée RSA puis
            //  on envoie une demande de pseudo au serveur

            // TODO
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
                                                    cstate->type_salon_actuel;
        strcpy(cstate->msg_waiting_acq[id_new_msg].destination,
                                                    cstate->destination);
        cstate->msg_waiting_acq[id_new_msg].proxy_client_socket = -1;
        strcpy(cstate->msg_waiting_acq[id_new_msg].msg, msg);
        cstate->msg_waiting_acq[id_new_msg].taille_msg = msg_len;

        tcp_connection_send_struct_message(con, con->sockfd,
                                        cstate->msg_waiting_acq[id_new_msg]);
    }

}


void on_msg_client(TcpConnection* con, SOCKET sock, 
                   Message msg, size_t msg_len,
                   void* custom_args){
    (void)con;
    (void)sock;
    (void)msg_len;
    ClientState* cstate = custom_args;
    (void)cstate;
    
    printf("Message received: %s\n", msg.msg);

}


// Initialise le client_state
void init_client_state(ClientState* client_state){
    // Init pseudo
    memset(client_state->pseudo, 0, T_NOM_MAX);
    strcpy(client_state->pseudo, "");

    // Init bools
    client_state->connected = false;
    client_state->attente_confirmation_pseudo = false;

    // Init salon
    client_state->type_salon_actuel = 0;
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
