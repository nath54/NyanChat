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


// Sending a message to the server
void client_send_message(TcpConnection* con,
                         ClientState* cstate,
                         char msg[MAX_MSG_LENGTH],
                         size_t msg_len)
{
    int id_new_msg = find_next_msg_id(cstate);

    cstate->msg_waiting_ack[id_new_msg].msg_type = MSG_STD_CLIENT_SERVER;
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


void client_process_message(TcpConnection* con, 
                            ClientState* cstate
){

    if (strlen(cstate->pseudo) == 0){
        // No nickname, not connected, so either:
        //  - waiting for user input for the nickname
        //  - waiting for the server to confirm the nickname (nothing to do)

        if (!cstate->waiting_pseudo_confirmation){
            // We do not wait for the server, we wait for user input
            // msg is supposed to contain the nickname requested by the client

            // We create a code then
            //  we send a nickname request to the server


            if (cstate->input_length < MIN_NAME_LENGTH){
                printf("Pseudo trop court, "
                       "doit avoir une taille entre 4 et 64 !\n"
                       "\nEntrez votre pseudo > ");
            }
            else {
                // Registration (temporary or not) of the pseudo
                strcpy(cstate->pseudo, cstate->input);

                // Creation of the directory for codes files if it doesn't exist
                struct stat st = {0};

                if (stat(PATH_CLI_CODES, &st) == -1){
                    CHK( mkdir(PATH_CLI_CODES, 0700) );
                }

                // Code directory for given pseudo
                char path_code_pseudo[MAX_NAME_LENGTH + 100] = PATH_CLI_CODES;
                CHKN( strcat(path_code_pseudo, cstate->input) );

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
                else{
                    // Need to load the code
                    size_t code_length;
                    CHK( read_file(path_code_pseudo,
                                   &(cstate->connection_code),
                                   &code_length) );
                    //
                    if(code_length != CODE_LENGTH){
                        fprintf(stderr,
                                "Error: code length error : %ld != %d!\n",
                                code_length, CODE_LENGTH);
                        exit(EXIT_FAILURE);
                    }
                }

                // Send connection request to the server
                init_empty_message(&(cstate->msg_waiting_ack[0]));
                cstate->msg_waiting_ack[0].msg_type = MSG_CONNECTION_CLIENT;
                cstate->msg_waiting_ack[0].msg_id = 0;
                strcpy(cstate->msg_waiting_ack[0].src_pseudo, cstate->input);
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
        // Connected right, messages can be sended normaly
        client_send_message(con, cstate, cstate->input, cstate->input_length);
    }

}


void on_stdin_client(TcpConnection* con,
                    char buffer[MAX_MSG_LENGTH],
                    size_t buf_len,
                    void* custom_args)
{
    ClientState* cstate = custom_args;

    if(buf_len == 0){
        return;
    }

    // Input gestion
    if(cstate->user_focus == FOCUS_INPUT){
        if(buffer[0] == '\x1b'){
            if(buffer[1] == SPECIAL_CHAR_ARROW){
                // TODO
            } else if(buffer[1] == SPECIAL_CHAR_KEYS){
                if(buffer[2] == K_ENTER){
                    client_process_message(con, cstate);
                    cstate->input_length = 0;
                }
                else if(buffer[2] == K_TABULATION){
                    cstate->user_focus = FOCUS_LEFT_PANEL;
                    cstate->hard_focus = false;
                }
                // TODO
            }
        }
        else{
            // We write the character in the input if possible
            if(cstate->input_length >= MAX_MSG_LENGTH){
                // Do nothing, input has reached max length!
                return;
            }
            else if(cstate->input_cursor == cstate->input_length){
                // Easy case
                cstate->input[cstate->input_cursor] = buffer[0];
                cstate->input_cursor++;
                cstate->input_length++;
            }
            else{
                // Shit case, have to shift at the right of the cursor
                for(int i = cstate->input_cursor; i<cstate->input_length; i++){
                    cstate->input[i+1] = cstate->input[i];   
                }
                // After the shift, we can write the character to the buffer
                cstate->input[cstate->input_cursor] = buffer[0];
                cstate->input_cursor++;
                cstate->input_length++;
            }
        }
    }
    else{
        if(cstate->hard_focus){
            // Hard focus
            if(buffer[0] == '\x1b'){
                if(buffer[1] == SPECIAL_CHAR_ARROW){
                    if(buffer[2] == ARROW_TOP){

                    }
                    else if(buffer[2] == ARROW_BOTTOM){

                    }

                    // TODO
                }
                else if(buffer[1] == SPECIAL_CHAR_KEYS){
                    if(buffer[2] == K_ESCAPE){
                        cstate->hard_focus = false;
                    }
                    else if(buffer[2] == K_TABULATION){
                        cstate->user_focus = FOCUS_INPUT;
                        cstate->hard_focus = false;
                    }
                }
            }
        }
        else{
            // Soft Focus / Panel Navigation
            if(buffer[0] == '\x1b'){
                if(buffer[1] == SPECIAL_CHAR_ARROW){
                    if(buffer[2] == ARROW_TOP){
                        if(cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL){
                            cstate->user_focus = FOCUS_RIGHT_TOP_PANEL;
                        }
                    }
                    else if(buffer[2] == ARROW_BOTTOM){
                        if(cstate->user_focus == FOCUS_RIGHT_TOP_PANEL){
                            cstate->user_focus = FOCUS_RIGHT_BOTTOM_PANEL;
                        }
                    }
                    else if(buffer[2] == ARROW_LEFT){
                        if(cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL ||
                        cstate->user_focus == FOCUS_RIGHT_TOP_PANEL)
                        {
                            cstate->user_focus = FOCUS_LEFT_PANEL;    
                        }
                    }
                    else if(buffer[2] == ARROW_RIGHT){
                        if(cstate->user_focus == FOCUS_LEFT_PANEL){
                            cstate->user_focus = FOCUS_RIGHT_TOP_PANEL;
                        }
                    }

                    // TODO
                }
                else if(buffer[1] == SPECIAL_CHAR_KEYS){

                    if(buffer[2] == K_ENTER){
                        cstate->hard_focus = true;   
                    }
                    else if(buffer[2] == K_TABULATION){
                        cstate->user_focus = FOCUS_INPUT;
                        cstate->hard_focus = false;
                    }
                }
            }
        }
    }

    //

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
        // We check that we have either:
        //   - an error -> unusable pseudo
        //   - an encoded message from the server
        //       that will have to be decoded with our private key and sent back to it
        //   - a positive acknowledgment indicating that the message has been
        //       decoded correctly, and therefore that we are well connected
        // Otherwise, we ignore it, we are not supposed to receive anything else

        // Connection code path
        char path_code_pseudo[MAX_NAME_LENGTH + 100] = PATH_CLI_CODES;
        CHKN( strcat(path_code_pseudo, cstate->pseudo) );

        // Error gestion
        if(msg->msg_type == MSG_ERROR){
            // This pseudonym is not usable, so the user must be informed
            // and asked to enter another one.
            // The connection code files will also need to be deleted.

            printf("\033[31mError, this pseudo is already taken, "
                    "please choose another pseudo!\033[m\nPseudo : ");

            // CONNECTION CODE FILES REMOVAL
            remove(path_code_pseudo);

            // Pseudo reset
            strcpy(cstate->pseudo, "");
            cstate->waiting_pseudo_confirmation = false;
            
            // If an error is sent when the decoded message is returned,
            //   this message must also be cleaned up

            // Test bad ACK
            if(msg->msg_id >= 0 &&
               (size_t)msg->msg_id < cstate->nb_msg_waiting_ack &&
               cstate->msg_waiting_ack[msg->msg_id].msg_type != MSG_NULL
            ){
                // We clean the waiting message
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
            // We do nothing, we are not supposed to get here
        }
    }
    else if(cstate->connected){
        // We are well connected, so we can receive normally the messages

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
                // Have to resend the message
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
                // Cleaning the awaiting message
                init_empty_message(&(cstate->msg_waiting_ack[msg->msg_id]));
                break;

            default:
                break;
        }

    }
}


// Initialising the client_state
void init_client_state(ClientState* cstate){
    
    // Init pseudo
    CHKN( memset(cstate->pseudo, '\0', MAX_NAME_LENGTH) );

    // Init bools
    cstate->connected = false;
    cstate->waiting_pseudo_confirmation = false;

    // Init connection code
    cstate->connection_code = NULL;

    // Init salon
    cstate->type_current_dest = 0;
    CHKN( memset(cstate->destination, '\0', MAX_NAME_LENGTH) ); 

    // Init msg_waiting_ack
    cstate->tot_msg_waiting_ack = 10;
    cstate->nb_msg_waiting_ack = 0;
    CHKN( cstate->msg_waiting_ack = calloc(
                    cstate->tot_msg_waiting_ack,
                    sizeof(Message)) );
    //
    for (size_t i=0; i<cstate->tot_msg_waiting_ack; i++){
        cstate->msg_waiting_ack[i].msg_type = -1;
    }

    
    // Init connected_clients
    cstate->tot_connected_clients = 10;
    cstate->nb_connected_clients = 0;
    CHKN( cstate->connected_clients = calloc(
                cstate->tot_connected_clients,
                sizeof(ConClient)) );
    //
    for (size_t i=0; i<cstate->tot_connected_clients; i++){
        CHKN( memset(cstate->connected_clients[i].pseudo, '\0', MAX_NAME_LENGTH) );
    }

    // Init channels
    cstate->tot_channels = 10;
    cstate->nb_channels = 0;
    CHKN( cstate->channels = calloc(
                cstate->tot_channels,
                sizeof(Channel)) );
    //
    for (size_t i=0; i<cstate->tot_channels; i++){
        CHKN( memset(cstate->channels[i].name, '\0', MAX_NAME_LENGTH) );
    }

    //
    cstate->user_focus = FOCUS_INPUT;

    //
    cstate->disp_msgs_cursor = 0;

    //
    CHKN( memset(cstate->input, '\0', MAX_MSG_LENGTH) );
    cstate->input_length = 0;
    cstate->input_cursor = 0;

    //
    cstate->win_height = 0;
    cstate->win_height = 0;

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
