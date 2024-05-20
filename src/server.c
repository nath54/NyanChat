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
#include "../include/codes_detection_correction.h"
#include "../include/server.h"


void gen_positive_acq_from_msg(Message* msg, Message* acq){
    init_empty_message(acq);
    acq->type_msg = MSG_ACQ_POS;
    acq->id_msg = msg->id_msg;
}


void gen_negative_acq_from_msg(Message* msg, Message* acq){
    init_empty_message(acq);
    acq->type_msg = MSG_ACQ_NEG;
    acq->id_msg = msg->id_msg;
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

    if(msg->type_msg == MSG_NORMAL_CLIENT_SERVER && msg->taille_msg >= 10){
        
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
        Message acq;
        if(msg_bon){
            // Acquittement positif: on a bien reçu le message
            gen_negative_acq_from_msg(msg, &acq);

            tcp_connection_send_struct_message(
                con,
                sock,
                &acq
            );
        }
        else{
            // Acquittement négatif: on n'a pas bien reçu le message
            gen_negative_acq_from_msg(msg, &acq);

            tcp_connection_send_struct_message(
                con,
                sock,
                &acq
            );
        }


    }

    printf("Message reçu : \"%s\"\n", msg->msg);
}


void on_stdin_server(TcpConnection* con,
                     char msg[T_MSG_MAX], size_t msg_len,
                     void* custom_args)
{
    (void)con;
    (void)msg_len;
    ServerState* sstate = custom_args;
    (void)sstate;

    // On affiche juste l'entrée reçue
    printf("Message écrit : \"%s\"\n", msg);
}


void init_server_state(ServerState* sstate){
    (void)sstate;
    
    // TODO: compléter cette fonction
}

void free_server_state(ServerState* sstate){
    (void)sstate;

    // TODO: compléter cette fonction
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
