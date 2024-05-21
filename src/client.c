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

// #include "tcp_connection.h"
#include "lib_ansi.h"
#include "lib_chks.h"
#include "client.h"
#include "useful_lib.h"

#define PATH_CLI_CODES "pseudo_code_clients/"


// Find the next free slots of the msg_waiting_ack
int find_next_msg_id(ClientState* cstate){
    size_t first_free = 0;
    if (cstate->nb_msg_waiting_ack == cstate->tot_msg_waiting_ack){
        first_free = cstate->tot_msg_waiting_ack;
        cstate->tot_msg_waiting_ack *= 2;
        cstate->msg_waiting_ack = realloc(
                cstate->msg_waiting_ack,
                sizeof(Message)*cstate->tot_msg_waiting_ack
        );
        if (cstate->msg_waiting_ack == NULL){
            fprintf(stderr, "Erreur Realloc!\n");
            exit(EXIT_FAILURE);
        }
        //
        for (size_t i = first_free; i<cstate->tot_msg_waiting_ack; i++){
            cstate->msg_waiting_ack[i].msg_type = MSG_NULL;
        }
    }
    //
    for (size_t i=first_free; i<cstate->tot_msg_waiting_ack; i++){
        if(cstate->msg_waiting_ack[i].msg_type == MSG_NULL){
            return i;
        }
    }
    //
    fprintf(stderr, "Erreur programme, panic!\n");
    exit(EXIT_FAILURE);
}


void client_send_message(TcpConnection* con,
                         ClientState* cstate,
                         char msg[MAX_MSG_LENGTH],
                         size_t msg_len)
{
    int id_new_msg = find_next_msg_id(cstate);

    cstate->msg_waiting_ack[id_new_msg].msg_type = 1;
    cstate->msg_waiting_ack[id_new_msg].msg_id = id_new_msg;
    strcpy(cstate->msg_waiting_ack[id_new_msg].src_pseudo,
                                                cstate->pseudo);
    cstate->msg_waiting_ack[id_new_msg].dst_flag = 
                                                cstate->type_current_dest;
    strcpy(cstate->msg_waiting_ack[id_new_msg].dst,
                                                cstate->destination);
    cstate->msg_waiting_ack[id_new_msg].proxy_client_socket = MSG_NULL;
    strcpy(cstate->msg_waiting_ack[id_new_msg].msg, msg);
    cstate->msg_waiting_ack[id_new_msg].msg_length = msg_len;

    cstate->nb_msg_waiting_ack += 1;

    tcp_connection_message_send(con, con->sockfd,
                                &(cstate->msg_waiting_ack[id_new_msg]));
}


void on_stdin_client(TcpConnection* con,
                    char msg[MAX_MSG_LENGTH],
                    size_t msg_len,
                    void* custom_args)
{
    ClientState* cstate = custom_args;

    if (strlen(cstate->pseudo) == 0){
        // No nickname, not connected, so either:
        //  - waiting for user input for the nickname
        //  - waiting for the server to confirm the nickname (nothing to do)

        if (!cstate->waiting_pseudo_confirmation){
            // On n'attend pas le serveur, on attend l'entrée utilisateur
            // msg est censé contenir le pseudo demandé par le client

            // On crée un code puis
            //  on envoie une demande de pseudo au serveur

            if (msg_len < MIN_NAME_LENGTH){
                printf("Pseudo trop court, "
                       "doit avoir une taille entre 4 et 64 !\n"
                       "\nEntrez votre pseudo > ");
            }
            else {
                // Registration (temporary or not) of the pseudo
                strcpy(cstate->pseudo, msg);

                // Creation of the directory for codes files if it doesn't exist
                struct stat st = {0};

                if (stat(PATH_CLI_CODES, &st) == -1){
                    CHK( mkdir(PATH_CLI_CODES, 0700) );
                }

                // Code directory for given pseudo
                char path_code_pseudo[MAX_NAME_LENGTH + 100] = PATH_CLI_CODES;
                CHKN( strcat(path_code_pseudo, msg) );

                // Login test (first login with this pseudo)
                if (stat(path_code_pseudo, &st) == -1) {
                    // Need to create code file

                    // Code creation
                    cstate->connection_code = generate_random_code(CODE_LENGTH);

                    // Code storage
                    FILE* fcode;
                    CHKN( fcode = fopen(path_code_pseudo, "w") );
                    CHKN( cstate->connection_code );
                    fwrite(cstate->connection_code,
                           sizeof(char), CODE_LENGTH, fcode);
                    CHK( fclose(fcode) );
                }

                // Send connection request to the server
                init_empty_message(&(cstate->msg_waiting_ack[0]));
                cstate->msg_waiting_ack[0].msg_type = MSG_CONNECTION_CLIENT;
                cstate->msg_waiting_ack[0].msg_id = 0;
                strcpy(cstate->msg_waiting_ack[0].src_pseudo, msg);
                strcpy(cstate->msg_waiting_ack[0].msg, cstate->connection_code);
                cstate->msg_waiting_ack[0].msg_length = CODE_LENGTH;
                cstate->nb_msg_waiting_ack += 1;
                //
                tcp_connection_message_send(con, con->sockfd,
                                            &(cstate->msg_waiting_ack[0]));
                //
                cstate->waiting_pseudo_confirmation = true;
            }
        }
    }
    else if (cstate->connected){
        // Connected right, messaages caan be sended normaly
        client_send_message(con, cstate, msg, msg_len);
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
    
    // printf("Message received: %s\n", msg->msg);

    if(cstate->waiting_pseudo_confirmation){
        // On vérifie que l'on a bien soit:
        //   - une erreur -> pseudo non utilisable
        //   - un message encodé du serveur
        //       qu'il faudra décoder avec notre clé privée et lui renvoyer
        //   - un acquittement positif qui indique que le message a bien été
        //       décodé, et donc qu'on est bien connecté
        // Sinon, on ignore, on est pas censé recevoir autre chose

        // Connection code path
        char path_code_pseudo[MAX_NAME_LENGTH + 100] = PATH_CLI_CODES;
        CHKN( strcat(path_code_pseudo, cstate->pseudo) );

        // Gestion erreur
        if(msg->msg_type == MSG_ERROR){
            // Pseudo non utilisable, il faut donc le dire à l'utilisateur
            //  et lui demander d'en rentrer un autre
            //  il faudra donc aussi supprimer les fichiers de codes de connexion

            printf("\033[31mError, this pseudo is already taken, "
                    "please choose another pseudo!\033[m\nPseudo : ");

            // SUPPRESSION DES FICHIERS DE CODE DE CONNEXION:
            remove(path_code_pseudo);

            // Réinitialisation du pseudo
            strcpy(cstate->pseudo, "");
            cstate->waiting_pseudo_confirmation = false;

            // Si erreur envoyée sur retour du message décodé,
            //   il faut aussi nettoyer ce message
            // Test bad ACK
            if(msg->msg_id >= 0 &&
               (size_t)msg->msg_id < cstate->nb_msg_waiting_ack &&
               cstate->msg_waiting_ack[msg->msg_id].msg_type != MSG_NULL
            ){
                // On nettoie le message qui attend
                init_empty_message(&(cstate->msg_waiting_ack[msg->msg_id]));
            }
        }
        else if(msg->msg_type == MSG_WELL_CONNECTED){
            cstate->connected = true;
            cstate->waiting_pseudo_confirmation = false;
            printf("Bien connecté au serveur!\n");

            // TODO: récupérer les messages des salons, etc...
        }
        else{
            // On ne fait rien, on n'est pas censé arriver ici
        }
    }
    else if(cstate->connected){
        // On est bien connecté, donc on reçoit les messages normalement

        switch (msg->msg_type)
        {
            case MSG_SERVER_CLIENT:
                printf("Received message from %s : \"%s\"\n",
                                        msg->src_pseudo, msg->msg);
                break;

            case MSG_ERROR:
                printf("\033[31mError from server: %s\033[m\n", msg->msg);
                break;

            case MSG_ACK_NEG:
                // Test bad ACK
                if(msg->msg_id < 0 ||
                   (size_t)msg->msg_id > cstate->nb_msg_waiting_ack ||
                   cstate->msg_waiting_ack[msg->msg_id].msg_type == MSG_NULL
                ){
                    fprintf(stderr, "\033[31mError, "
                            "bad negative ack from server\033[m\n");
                    return;
                }
                // Il faut renvoyer le message
                tcp_connection_message_send(con, con->sockfd,
                                    &(cstate->msg_waiting_ack[msg->msg_id]));
                break;
            
            case MSG_ACK_POS:
                // Test bad ACK
                if(msg->msg_id < 0 ||
                   (size_t)msg->msg_id > cstate->nb_msg_waiting_ack ||
                   cstate->msg_waiting_ack[msg->msg_id].msg_type == MSG_NULL
                ){
                    fprintf(stderr, "\033[31mError, "
                            "bad negative ack from server\033[m\n");
                    return;
                }
                // On nettoie le message qui attend
                init_empty_message(&(cstate->msg_waiting_ack[msg->msg_id]));
                break;

            default:
                break;
        }

    }
}


// Initialise le client_state
void init_client_state(ClientState* client_state){
    // Init pseudo
    memset(client_state->pseudo, 0, MAX_NAME_LENGTH);
    strcpy(client_state->pseudo, "");

    // Init bools
    client_state->connected = false;
    client_state->waiting_pseudo_confirmation = false;

    // Init salon
    client_state->type_current_dest = 0;
    memset(client_state->destination, 0, MAX_NAME_LENGTH);
    strcpy(client_state->destination, "");

    // Init msg_waiting_ack
    client_state->tot_msg_waiting_ack = 10;
    client_state->nb_msg_waiting_ack = 0;
    client_state->msg_waiting_ack = calloc(client_state->tot_msg_waiting_ack,
                                           sizeof(Message));
    //
    for (size_t i=0; i<client_state->tot_msg_waiting_ack; i++){
        client_state->msg_waiting_ack[i].msg_type = -1;
    }
    //
    if (client_state->msg_waiting_ack == NULL){
        fprintf(stderr, "Error malloc! \n");
        exit(EXIT_FAILURE);
    }

    // 
    client_state->connection_code = NULL;

    // First thing to do: asking for client pseudo
    print_rainbow("Welcome to NyanChat!\n");
    printf("\nEnter your name > ");
}


void free_client_state(ClientState* client_state){
    free(client_state->msg_waiting_ack);
}


int main(int argc, char* argv[]) {
    
    ClientState client_state;
    TcpConnection con;

    // Check arguments
    if (argc != 3){
        printf("Usage: %s ip_proxy port_proxy\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* ip_proxy = argv[1];
    int port_proxy = atoi(argv[2]);

    if (port_proxy < 5000 || port_proxy > 65000){
        fprintf(stderr, "Bad value of port_proxy : %d !\n", port_proxy);
        exit(EXIT_FAILURE);
    }

    init_client_state(&client_state);
    tcp_connection_client_init(&con, ip_proxy, port_proxy, -1);

    tcp_connection_mainloop(&con,
                            on_msg_client, &client_state,
                            on_stdin_client, &client_state);

    tcp_connection_close(&con);
    free_client_state(&client_state);

    return 0;
}
