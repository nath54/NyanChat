#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
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
#include "lib_chks.h"

#include "useful_lib.h"

#define PATH_SRV_CODES "pseudo_code_server/"


// Create a Client struct
Client* create_client(char pseudo[MAX_NAME_LENGTH], int id_poll_socket){
    Client* c = calloc(1, sizeof(Client));
    if(c == NULL){
        fprintf(stderr, "\033[31mError allocation\033[m\n");
    }

    c->connected = false;
    strcpy(c->pseudo, pseudo);
    c->last_activity = time(NULL);
    c->id_poll_socket_proxy = id_poll_socket;

    return c;
}


// Free a client structure
void free_client(Client** c){
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

    sstate->hm_pseudo_to_id = hashmap_create(NB_MAX_CONNECTED_CLIENTS * 2);

    //
    struct stat st = {0};
    if (stat(PATH_SRV_CODES, &st) == -1){
        CHK( mkdir(PATH_SRV_CODES, 0700) );
    }
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
    ack->proxy_client_socket = msg->proxy_client_socket;
}


// Generate a negative acknowledgement from a message
void gen_negative_ack_from_msg(Message* msg, Message* ack){
    init_empty_message(ack);
    ack->msg_type = MSG_ACK_NEG;
    ack->msg_id = msg->msg_id;
    ack->proxy_client_socket = msg->proxy_client_socket;
}


// Function that processes received messages
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

        // Recherche de données sur un client avec le pseudo demandé
        Client* cli = NULL;
        if( hashmap_find(sstate->hm_pseudo_to_id, msg->src_pseudo) != -1 ){
            // On a trouvé une valeur
            uint idx_client = hashmap_get(sstate->hm_pseudo_to_id,
                                          msg->src_pseudo);
            if(idx_client < NB_MAX_CONNECTED_CLIENTS){
                cli = sstate->clients[idx_client];
                if(cli == NULL){
                    // Mauvaise valeur, on va la supprimer
                    hashmap_remove(sstate->hm_pseudo_to_id, msg->src_pseudo);
                }
            }
            else{
                // Mauvaise valeur, on va la supprimer
                hashmap_remove(sstate->hm_pseudo_to_id, msg->src_pseudo);
            }
        }

        // Test if doesn't exists a code stored for this pseudo
        char path_key[MAX_NAME_LENGTH + 100] = PATH_SRV_CODES;
        CHKN( strcat(path_key, msg->src_pseudo) );
        struct stat st = {0};

        if (stat(path_key, &st) != -1){
            // S'il y en a une, on la compare avec celle envoyée
            char* found_code;
            size_t key_length;
            CHK( read_file(path_key, &found_code, &key_length) );

            if(strcmp(msg->msg, found_code) != 0){
                // Si les deux codes sont différentes,
                //   envoi d'un msg d'erreur au client qui veut se connecter
                Message error_msg;
                init_empty_message(&error_msg);
                error_msg.msg_type = MSG_ERROR;
                error_msg.proxy_client_socket = msg->proxy_client_socket;
                strcpy(error_msg.msg, ERROR_MSG_CON_DIFF_KEYS);
                tcp_connection_message_send(con, sock, &error_msg);
                return;

            } else {
                // Sinon, on teste si l'utilisateur a déjà une session active
                if(cli != NULL && cli->connected == true){
                    // Si oui, on test si le client est inactif
                    //   depuis un certain temps
                    if(time(NULL) - cli->last_activity >= TIMEOUT_INACTIVE){
                        // Si oui, on jarte le client déjà connecté
                        //       -> (on remplace id_poll_socket_proxy)
                        //           une session client est identifié par:
                        //                - son pseudo 
                        //                - et son id de socket du proxy
                        // mais avant, on va quand même tester les clés
                        
                    }
                    else{
                        // Sinon, on envoie un message d'erreur
                        Message error_msg;
                        init_empty_message(&error_msg);
                        error_msg.msg_type = MSG_ERROR;
                        error_msg.proxy_client_socket = msg->proxy_client_socket;
                        strcpy(error_msg.msg, ERROR_MSG_STILL_ACTIVE_CLIENT);
                        tcp_connection_message_send(con, sock, &error_msg);
                        return;
                    }
                }
            }

            free(found_code);
        }

        // On va s'occuper du client, en créer un autre, ou reset l'existant
        if(cli != NULL){
            // il faut "reset" les données du client
            cli->id_poll_socket_proxy = msg->proxy_client_socket;
            cli->last_activity = time(NULL);
        }
        else{
            // il faut créer un client associé à ce pseudo

            // On recherche la première case vide du tableau des clients
            int new_cli_idx = -1;
            for(int i=0; i<NB_MAX_CONNECTED_CLIENTS; i++){
                if(sstate->clients[i] == NULL){
                    new_cli_idx = i;
                    break;
                }
            }
            if(new_cli_idx == -1){
                // Message d'erreur, plus de clients possibles
                Message error_msg;
                init_empty_message(&error_msg);
                error_msg.msg_type = MSG_ERROR;
                error_msg.proxy_client_socket = msg->proxy_client_socket;
                strcpy(error_msg.msg, ERROR_ALREADY_MAX_CLIENTS);
                tcp_connection_message_send(con, sock, &error_msg);
                return;
            }

            // On crée le client
            sstate->clients[new_cli_idx] = create_client(msg->src_pseudo,
                                                msg->proxy_client_socket);
            cli = sstate->clients[new_cli_idx];
            //
            hashmap_insert(sstate->hm_pseudo_to_id,
                           cli->pseudo, new_cli_idx);
        }

        /*
        The client is now connected
        */

        // Storing the code associated to the pseudo
        FILE* fstorecode = fopen(path_key, "w");
        fwrite(msg->msg, sizeof(char), msg->msg_length, fstorecode);
        fclose(fstorecode);

        cli->connected = true;

        // On envoie un message pour indiquer la bonne connection au client
        Message connected_msg;
        init_empty_message(&connected_msg);
        connected_msg.msg_type = MSG_WELL_CONNECTED;
        connected_msg.proxy_client_socket = msg->proxy_client_socket;
        tcp_connection_message_send(con, sock, &connected_msg);
    }
    else if(msg->msg_type == MSG_STD_CLIENT_SERVER &&
            msg->msg_length >= 10
    ){
        
        printf("Received (from: %s) : \"%s\" (%d)\n",
               msg->src_pseudo, msg->msg, msg->msg_length);

        // test detection d'erreurs
        bool msg_bon = true;
        switch (code_detect_error(msg))
        {
            case 0:  // No detected error
                break;
            
            case 1:
                if(code_correct_error(msg) != 0)
                    msg_bon = false;
                break;

            default:
                msg_bon = false;
                break;
        }

        // Acknowledgment sending
        Message ack;
        if (msg_bon){
            // Positive acknowledgment: we received the message correctly
            gen_negative_ack_from_msg(msg, &ack);

            tcp_connection_message_send(con, sock, &ack);
        }
        else {
            // Negative acknowledgment: we did not receive the message correctly
            gen_negative_ack_from_msg(msg, &ack);

            tcp_connection_message_send(con, sock, &ack);
        }

    }
    else if(msg->msg_type == MSG_STD_CLIENT_SERVER){  // Tout piti msg
        printf("Received (from: %s) : \"%s\" (%d)\n",
               msg->src_pseudo, msg->msg, msg->msg_length);
    }
    else{

    }
}


void on_stdin_server(TcpConnection* con,
                     char msg[MAX_MSG_LENGTH], size_t msg_len,
                     void* custom_args)
{
    (void)con;
    (void)msg_len;
    ServerState* sstate = custom_args;
    (void)sstate;

    // Print the received input
    printf("Message écrit : \"%s\"\n", msg);

    // Test de commandes
    //   - Commande pour quitter le serveur
    if(strcmp(msg, "/quit") == 0){
        con->end_connection = true;
    }
}


int main(int argc, char **argv)
{
    ServerState server_state;
    TcpConnection con;

    // Check of the number of arguments on the command line
    if(argc != 2)
    {
        printf("Usage: %s port_local\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    // Initialisation of the tcp connection on the server side
    init_server_state(&server_state);
    tcp_connection_server_init(&con, "127.0.0.1", port, 20, -1);

    // Main loop of the tcp connection
    tcp_connection_mainloop(&con,
                            on_msg_received, &server_state,
                            on_stdin_server, &server_state);

    // Closure of the connection
    tcp_connection_close(&con);
    free_server_state(&server_state);

    return 0;
}
