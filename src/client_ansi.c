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
#include <termios.h>
#include <ctype.h>

// #include "tcp_connection.h"
#include "lib_ansi.h"
#include "lib_chks.h"
#include "client.h"
#include "useful_lib.h"

#define PATH_CLI_CODES "pseudo_code_clients/"


/*
    ------------------------ Global Variables ------------------------
*/

// To save the original state of the terminal to restore it at the end
termios_t orig_termios;


/*
    ------------------------ Side Display Functions ------------------------
*/





/*
    ------------------------ Client Display Functions ------------------------
*/


void display_client_connection_window(ClientState* cstate)
{
    //
    hide_cursor();
    clean_terminal();
    set_cursor_position(0, 0);

    set_screen_border(cstate->win_width, cstate->win_height);

    int x_logo = (cstate->win_width - cstate->logo_connection->tx) / 2;
    int y_logo = 3;
    print_ascii_art_with_gradients(x_logo, y_logo,
                                   cstate->logo_connection,
                                   CYAN, VIOLET, ORANGE);

    int y_fin_logo = y_logo + cstate->logo_connection->ty - 3;

    char welcome_txt[] = "Welcome in NyanChat!";
    print_centered_text(welcome_txt, strlen(welcome_txt),
                        2, cstate->win_width-3, y_fin_logo + 4);

    char pseudo_txt[] = "Please enter your pseudo to continue:";
    print_centered_text(pseudo_txt, strlen(pseudo_txt),
                        2, cstate->win_width-3, y_fin_logo + 6);
    
    int x_inp = (cstate->win_width - MAX_NAME_LENGTH - 6) / 2;
    int tx_inp = MAX_NAME_LENGTH;

    print_horizontal_line('-', x_inp, x_inp + tx_inp + 4, y_fin_logo + 8);
    print_horizontal_line('-', x_inp, x_inp + tx_inp + 4, y_fin_logo + 10);
    set_cursor_position(x_inp, y_fin_logo + 9);
    printf(">");
    set_cursor_position(x_inp + tx_inp + 4, y_fin_logo + 9);
    printf("|");

    set_cursor_position(x_inp + 2, y_fin_logo + 9);
    if(cstate->input_length > 0)
        { printf("%s", cstate->input); }

    cstate->cursor_x = x_inp + 2 + cstate->input_cursor;
    cstate->cursor_y = y_fin_logo + 9;

    //
    set_cursor_position(cstate->cursor_x, cstate->cursor_y);
    show_cursor();
    force_buffer_prints();
}


void display_client_main_window(ClientState* cstate)
{
    //
    hide_cursor();
    clean_terminal();
    set_cursor_position(0, 0);

    int x_barriere_top = cstate->win_width - cstate->logo_main->tx - 1;
    int y_barriere_bottom = cstate->win_height - 4;
    int y_barriere_logo_right = 1 + cstate->logo_main->ty;

    // Main architecture
    set_screen_border(cstate->win_width, cstate->win_height);
    print_vertical_line('#', x_barriere_top, 1, y_barriere_bottom-1);
    print_horizontal_line('#', y_barriere_logo_right, x_barriere_top+1,
                          cstate->win_width-2);

    // -- logo --
    print_ascii_art_with_gradients(x_barriere_top+1, 2, cstate->logo_main,
                                   GREEN, YELLOW, RED);

    // -- menus --
    print_horizontal_line('#', x_barriere_top+1, cstate->win_width-1, 4);
    if (cstate->user_focus == FOCUS_RIGHT_TOP_PANEL) {
        set_bold();
        set_cl_fg(FOCUS_COLOR);
    }
    print_vertical_line('|', x_barriere_top+1, 5, 7);
    print_vertical_line('|', cstate->win_width-1, 5, 7);
    if (cstate->user_focus == FOCUS_INPUT)
        { reset_ansi(); }

    // TODO: display left / right arrow
    // TODO: display menu name

    // Transition menus - right panel
    set_cursor_position(x_barriere_top+1, 8);
    printf("#");
    set_cursor_position(cstate->win_width-1, 8);
    printf("#");
    if(cstate->user_focus == FOCUS_RIGHT_TOP_PANEL ||
       cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL
    ){
        set_bold();
        set_cl_fg(FOCUS_COLOR);
    }
    print_horizontal_line('-', x_barriere_top+2, cstate->win_width-2, 8);
    if(cstate->user_focus == FOCUS_RIGHT_TOP_PANEL ||
       cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL
    ){
        reset_ansi();
    }

    // Right pannel
    if(cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL){
        set_bold();
        set_cl_fg(FOCUS_COLOR);
    }
    print_horizontal_line('-', x_barriere_top+1, cstate->win_width-2, cstate->win_height-5);
    if(cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL){
        reset_ansi();
    }

    // TODO: content of the right panel + scroll

    // Messages pannel
    if(cstate->user_focus == FOCUS_LEFT_PANEL){
        set_bold();
        set_cl_fg(FOCUS_COLOR);
    }
    print_horizontal_line('-', 2, x_barriere_top-1, 2);
    print_horizontal_line('-', 2, x_barriere_top-1, cstate->win_height-5);
    if(cstate->user_focus == FOCUS_LEFT_PANEL){
        reset_ansi();
    }


    // TODO: content of the message panel + scroll


    // -- Bottom input --
    print_horizontal_line('#', 1, cstate->win_width-2, y_barriere_bottom);
    if(cstate->user_focus == FOCUS_INPUT){
        set_bold();
        set_cl_fg(FOCUS_COLOR);
    }
    print_vertical_line('|', 2, cstate->win_height-3, cstate->win_height-1);
    print_vertical_line('|', cstate->win_width-2, cstate->win_height-3, cstate->win_height-1);
    print_horizontal_line('-', 2, cstate->win_width - 2, cstate->win_height-3);
    print_horizontal_line('-', 2, cstate->win_width - 2, cstate->win_height-1);
    if (cstate->user_focus == FOCUS_INPUT)
        { reset_ansi(); }

    set_cursor_position(3, cstate->win_height-2);
    printf(">");
    if (cstate->input_length < cstate->win_width - 8)
        { printf(" %s", cstate->input); }

    // at the end, we set the cursor at the good input position, if input focus
    if (cstate->user_focus == FOCUS_INPUT) {
        if (cstate->input_length < cstate->win_width - 8)
            { cstate->cursor_x = 6+cstate->input_cursor; }
        else {
            cstate->cursor_x = cstate->win_width - 6;
            // TODO
        }
        cstate->cursor_y = cstate->win_height-2;
        //
        set_cursor_position(cstate->cursor_x, cstate->cursor_y);
        show_cursor();
    }

    force_buffer_prints();
}


void display_client_error_win_size(ClientState* cstate)
{
    clean_terminal();
    set_cursor_position(0, 0);
    printf("Terminal windows too small : (%d, %d)\n"
           "Please resize it at least (%d, %d)\n",
           cstate->win_width, cstate->win_height,
           MIN_TERMINAL_WIDTH, MIN_TERMINAL_HEIGHT);
    force_buffer_prints();
}




void display_client(ClientState* cstate)
{
    get_terminal_size(&(cstate->win_width), &(cstate->win_height));
    //
    if(cstate->win_width < MIN_TERMINAL_WIDTH ||
       cstate->win_height < MIN_TERMINAL_HEIGHT)
        { display_client_error_win_size(cstate); }
    //
    if (cstate->connected)
        { display_client_main_window(cstate); }
    else
        { display_client_connection_window(cstate); }
}




/*
    ------------------------ Send Message Functions ------------------------
*/


// Find the next free slots of the msg_waiting_ack
int find_next_msg_id(ClientState* cstate)
{
    size_t first_free = 0;
    if (cstate->nb_msg_waiting_ack == cstate->tot_msg_waiting_ack) {
        first_free = cstate->tot_msg_waiting_ack;
        cstate->tot_msg_waiting_ack *= 2;
        cstate->msg_waiting_ack = realloc(cstate->msg_waiting_ack,
                                  sizeof(Message)*cstate->tot_msg_waiting_ack);
        if (cstate->msg_waiting_ack == NULL) {
            fprintf(stderr, "Erreur Realloc!\n");
            exit(EXIT_FAILURE);
        }
        //
        for (size_t i = first_free; i<cstate->tot_msg_waiting_ack; i++)
            { cstate->msg_waiting_ack[i].msg_type = MSG_NULL; }
    }
    //
    for (size_t i=first_free; i<cstate->tot_msg_waiting_ack; i++){
        if(cstate->msg_waiting_ack[i].msg_type == MSG_NULL)
            { return i; }
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
    strcpy(cstate->msg_waiting_ack[id_new_msg].src_pseudo, cstate->pseudo);
    cstate->msg_waiting_ack[id_new_msg].dst_flag = cstate->type_current_dest;
    strcpy(cstate->msg_waiting_ack[id_new_msg].dst, cstate->destination);
    cstate->msg_waiting_ack[id_new_msg].proxy_client_socket = MSG_NULL;
    strcpy(cstate->msg_waiting_ack[id_new_msg].msg, msg);
    cstate->msg_waiting_ack[id_new_msg].msg_length = msg_len;

    cstate->nb_msg_waiting_ack += 1;

    tcp_connection_message_send(con, con->sockfd,
                                &(cstate->msg_waiting_ack[id_new_msg]));
}



/*
    ------------------------ On Event Functions ------------------------
*/


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


void on_client_input_connection_window(TcpConnection* con,
                                       ClientState* cstate,
                                       char msg[MAX_MSG_LENGTH],
                                       size_t msg_len)
{
    (void)con;
    (void)cstate;
    (void)msg;
    (void)msg_len;
}


void on_client_input_main_window(TcpConnection* con,
                                 ClientState* cstate,
                                 char msg[MAX_MSG_LENGTH],
                                 size_t msg_len)
{
    (void)con;
    (void)cstate;
    (void)msg;
    (void)msg_len;
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

    //
    if(buffer[0] == CTRL_KEY('q')){
        con->end_connection = true;
    }

    // Input gestion
    if(cstate->user_focus == FOCUS_INPUT){
        if(buffer[0] == '\x1b'){
            if(buffer[1] == SPECIAL_CHAR_ARROW){
                if(buffer[2] == ARROW_LEFT){
                    if(cstate->input_cursor > 0){
                        cstate->input_cursor--;
                    }
                }
                else if(buffer[2] == ARROW_RIGHT){
                    if(cstate->input_cursor < cstate->input_length - 1){
                        cstate->input_cursor++;
                    }
                }
            } else if(buffer[1] == SPECIAL_CHAR_KEYS){
                if(buffer[2] == K_ENTER){
                    client_process_message(con, cstate);
                    cstate->input_length = 0;
                }
                else if(buffer[2] == K_TABULATION && cstate->connected){
                    cstate->user_focus = FOCUS_LEFT_PANEL;
                    cstate->hard_focus = false;
                }
                else if(buffer[2] == K_BACKSPACE){
                    if(cstate->input_cursor > 0){
                        // have to shift at the left of the cursor
                        for(int i = cstate->input_cursor - 1; i<cstate->input_length - 1; i++){
                            cstate->input[i] = cstate->input[i+1];
                        }
                        //
                        cstate->input_cursor--;
                        cstate->input_length--;
                        cstate->input[cstate->input_length] = '\0';
                    }
                }
                else if(buffer[2] == K_DELETE){
                    if(cstate->input_length - cstate->input_cursor > 0){
                        // have to shift at the left of the cursor
                        for(int i = cstate->input_cursor; i<cstate->input_length - 1; i++){
                            cstate->input[i] = cstate->input[i+1];
                        }
                        //
                        cstate->input_length--;
                        cstate->input[cstate->input_length] = '\0';
                    }
                }
                // TODO
            }
        }
        else{
            // We write the character in the input if possible
            if(cstate->input_length >= MAX_MSG_LENGTH - 1){
                // Do nothing, input has reached max length!
            }
            else if(cstate->input_cursor == cstate->input_length){
                // Easy case
                cstate->input[cstate->input_cursor] = buffer[0];
                cstate->input_cursor++;
                cstate->input_length++;
                cstate->input[cstate->input_length] = '\0';
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
                cstate->input[cstate->input_length] = '\0';
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
    display_client(cstate);
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

    if (cstate->waiting_pseudo_confirmation) {
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
        if (msg->msg_type == MSG_ERROR) {
            // This pseudonym is not usable, so the user must be informed
            // and asked to enter another one.
            // The connection code files will also need to be deleted.

            // TODO : mettre ca dans une case d'erreru
            // printf("\033[31mError, this pseudo is already taken, "
            //         "please choose another pseudo!\033[m\nPseudo : ");

            // CONNECTION CODE FILES REMOVAL
            remove(path_code_pseudo);

            // Pseudo reset
            strcpy(cstate->pseudo, "");
            cstate->waiting_pseudo_confirmation = false;
            
            // If an error is sent when the decoded message is returned,
            //   this message must also be cleaned up

            // Test bad ACK
            if (msg->msg_id >= 0 &&
                (size_t)msg->msg_id < cstate->nb_msg_waiting_ack &&
                cstate->msg_waiting_ack[msg->msg_id].msg_type != MSG_NULL)
                // We clean the waiting message
                { init_empty_message(&(cstate->msg_waiting_ack[msg->msg_id])); }
        }
        else if (msg->msg_type == MSG_WELL_CONNECTED) {
            cstate->connected = true;
            cstate->waiting_pseudo_confirmation = false;
            display_client(cstate);

            // TODO: récupérer les messages des salons, etc...
        }
        else {
            // We do nothing, we are not supposed to get here
        }
    }
    else if (cstate->connected) {
        // We are well connected, so we can receive normally the messages

        switch (msg->msg_type)
        {
            case MSG_SERVER_CLIENT:
                // TODO: recevoir les messages
                // printf("Received message from %s : \"%s\"\n",
                //                         msg->src_pseudo, msg->msg);
                break;

            case MSG_ERROR:
                // TODO: recevoir les erreurs
                // printf("\033[31mError from server: %s\033[m\n", msg->msg);
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
                if (msg->msg_id < 0 ||
                    (size_t)msg->msg_id > cstate->nb_msg_waiting_ack ||
                    cstate->msg_waiting_ack[msg->msg_id].msg_type == MSG_NULL)
                {
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



/*
    ------------------------ Client State Functions ------------------------
*/


// Initialising the client_state
void init_cstate(ClientState* cstate)
{
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
    CHKN( cstate->msg_waiting_ack = calloc(cstate->tot_msg_waiting_ack,
                                           sizeof(Message)) );
    //
    for (size_t i=0; i<cstate->tot_msg_waiting_ack; i++)
        { cstate->msg_waiting_ack[i].msg_type = -1; }

    
    // Init connected_clients
    cstate->tot_connected_clients = 10;
    cstate->nb_connected_clients = 0;
    CHKN( cstate->connected_clients = calloc(
                cstate->tot_connected_clients,
                sizeof(ConClient)) );
    //
    for (size_t i=0; i<cstate->tot_connected_clients; i++) {
        CHKN( memset(cstate->connected_clients[i].pseudo, '\0',
                     MAX_NAME_LENGTH) );
    }

    // Init channels
    cstate->tot_channels = 10;
    cstate->nb_channels = 0;
    CHKN( cstate->channels = calloc(cstate->tot_channels, sizeof(Channel)) );
    //
    for (size_t i=0; i<cstate->tot_channels; i++)
        { CHKN( memset(cstate->channels[i].name, '\0', MAX_NAME_LENGTH) ); }

    //
    cstate->user_focus = FOCUS_INPUT;

    //
    cstate->disp_msgs_cursor = 0;

    //
    CHKN( memset(cstate->input, '\0', MAX_MSG_LENGTH) );
    cstate->input_length = 0;
    cstate->input_cursor = 0;
    cstate->cursor_x = 0;
    cstate->cursor_y = 0;

    //
    cstate->win_height = 0;
    cstate->win_height = 0;

    //
    cstate->logo_connection = load_ascii_art("res/logo_connection.txt");
    cstate->logo_main = load_ascii_art("res/logo_main.txt");
}


// Cleaning the client state
void free_cstate(ClientState* cstate)
{
    free(cstate->msg_waiting_ack);
    free_ascii_art(cstate->logo_connection);
    free_ascii_art(cstate->logo_main);
}


/*
    ------------------------ Main Functions ------------------------
*/


int main(int argc, char* argv[]) {
    
    fprintf(stderr, "test\n");

    ClientState cstate;
    TcpConnection con;

    // Check arguments
    if (argc != 3) {
        printf("Usage: %s ip_proxy port_proxy\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* ip_proxy = argv[1];
    int port_proxy = atoi(argv[2]);

    if (port_proxy < 5000 || port_proxy > 65000) {
        fprintf(stderr, "Bad value of port_proxy : %d !\n", port_proxy);
        exit(EXIT_FAILURE);
    }


    // Enabling raw terminal mode to get individually each characters
    enableRawMode(&orig_termios);

    init_cstate(&cstate);
    tcp_connection_client_init(&con, ip_proxy, port_proxy, -1);
    con.ansi_stdin = true;

    display_client(&cstate);

    tcp_connection_mainloop(&con, on_msg_client, &cstate,
                            on_stdin_client, &cstate);

    tcp_connection_close(&con);
    free_cstate(&cstate);

    return 0;
}
