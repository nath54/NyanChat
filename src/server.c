#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "tcp_connection.h"
#include "codes_detection_correction.h"
#include "server.h"


// Generate a random code to encode with client public key
char* generate_random_code(uint32 code_length){
    //
    char* code = calloc(code_length, sizeof(char));
    if(code == NULL){
        fprintf(stderr, "\033[31mError allocation\033[m\n");
    }
    //
    for(uint32 i=0; i<code_length - 1; i++){
        code[i] = 32 + rand() % 82;
    }
    //
    code[code_length - 1] = '\0';
    //
    return code;
}


// Create a Client struct
Client* create_client(char pseudo[MAX_NAME_LENGTH], int id_poll_socket){
    Client* c = calloc(1, sizeof(Client));
    if(c == NULL){
        fprintf(stderr, "\033[31mError allocation\033[m\n");
    }

    c->connected = false;
    strcpy(c->pseudo, pseudo);
    
    c->code_to_verify = generate_random_code(MAX_MSG_LENGTH / 2);
    c->id_poll_socket_proxy = id_poll_socket;
    c->waiting_code_response = true;
    
    c->last_activity = time(NULL);

    return c;
}


// Free a client structure
void free_client(Client** c){

    if((*c)->code_to_verify != NULL){
        free((*c)->code_to_verify);
    }

    free(*c);
    *c = NULL;
}


// Init the struct ServerState
void init_server_state(ServerState* sstate){
    (void)sstate;
    
    sstate->nb_clients = 0;
    for(int i=0; i<NB_MAX_CONNECTED_CLIENTS; i++){
        sstate->clients[i] = NULL;
    }

    hashmap_create(NB_MAX_CONNECTED_CLIENTS * 2);
}


// Free all the allocated values inside a ServerState struct
void free_server_state(ServerState* sstate){
    (void)sstate;
    
    for(int i=0; i<NB_MAX_CONNECTED_CLIENTS; i++){
        if(sstate->clients[i] != NULL){
            free_client(&(sstate->clients[i]));
        }
    }

    hashmap_free(sstate->hm_pseudo_to_id);
}


// Generate a positive acknowledgment from a message
void gen_positive_ack_from_msg(Message* msg, Message* ack){
    init_empty_message(ack);
    ack->msg_type = MSG_ACK_POS;
    ack->msg_id = msg->msg_id;
}


// Generate a negative acknowledgement from a message
void gen_negative_ack_from_msg(Message* msg, Message* ack){
    init_empty_message(ack);
    ack->msg_type = MSG_ACK_NEG;
    ack->msg_id = msg->msg_id;
}


// Fonction qui traite les messages reçus
void on_msg_received(TcpConnection* con, SOCKET sock,
                     Message* msg, size_t msg_length,
                     void* custom_args)
{
    (void)con;
    (void)sock;
    (void)msg_length;
    ServerState* sstate = custom_args;
    (void)sstate;

    if(strlen(msg->src_pseudo) < MIN_NAME_LENGTH){
        // Pas de pseudo, on bloque le message, on ne fait rien
        return;
    }

    if(msg->msg_type == MSG_CONNECTION_CLIENT){
        // On teste s'il n'y a pas déjà une clé publique associée au pseudo
        // TODO

        // S'il y en a une, on la compare avec celle envoyée
        // TODO

        // Si les deux clés sont différentes,
        //   envoi d'un message d'erreur au client qui essaie de se connecter
        // TODO

        // Sinon, on teste si l'utilisateur a déjà une session active
        //   Si oui, on teste si le client est inactif depuis un certain temps
        //       Si oui, on jarte le client déjà connecté
        //             -> (on remplace id_poll_socket_proxy)
        //                une session client est identifié par son pseudo 
        //                et par son id de socket du proxy
        // TODO

        //       Sinon, on envoie un message d'erreur
        // TODO

        // Si non déjà connecté, on encode un code random associé à ce client
        //      et on l'envoie, dans l'attente d'une réponse
        // TODO

        

    }

    if(msg->msg_type == MSG_NORMAL_CLIENT_SERVER && msg->msg_length >= 10){
        
        // test detection d'erreurs
        int res = code_detect_error(msg);
        bool msg_bon = true;

        if(res != 0){
            // Erreur détectée

            if(res == 1){ // On peut corriger?
                if(code_correct_error(msg) != 0){
                    msg_bon = false;
                }
            }
            else{
                msg_bon = false;
            }

        }

        // Envoi des acquittements
        Message ack;
        if(msg_bon){
            // Acquittement positif: on a bien reçu le message
            gen_negative_ack_from_msg(msg, &ack);

            tcp_connection_message_send(
                con,
                sock,
                &ack
            );
        }
        else{
            // Acquittement négatif: on n'a pas bien reçu le message
            gen_negative_ack_from_msg(msg, &ack);

            tcp_connection_message_send(
                con,
                sock,
                &ack
            );
        }

    }

    printf("Message reçu : \"%s\"\n", msg->msg);
}


void on_stdin_server(TcpConnection* con,
                     char msg[MAX_MSG_LENGTH], size_t msg_len,
                     void* custom_args)
{
    (void)con;
    (void)msg_len;
    ServerState* sstate = custom_args;
    (void)sstate;

    // On affiche juste l'entrée reçue
    printf("Message écrit : \"%s\"\n", msg);

    // Test de commandes
    //   - Commande pour quitter le serveur
    if(strcmp(msg, "/quit") == 0){
        con->end_connection = true;
    }
}


// Main
int main(int argc, char **argv)
{
    ServerState server_state;
    TcpConnection con;

    // verification du nombre d'arguments sur la ligne de commande
    if(argc != 2)
    {
        printf("Usage: %s port_local\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    // Initialisation de la connection tcp du côté serveur
    init_server_state(&server_state);
    tcp_connection_server_init(&con, "127.0.0.1", port, 20, -1);

    // Boucle principale de la connection tcp
    tcp_connection_mainloop(&con,
                            on_msg_received, &server_state,
                            on_stdin_server, &server_state);

    // fermeture de la connection
    tcp_connection_close(&con);
    free_server_state(&server_state);

    return 0;
}
