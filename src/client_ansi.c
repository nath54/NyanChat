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
    ------------------------ Side Libs Functions ------------------------
*/


// Add a message to the dynamic array of Message from the default channel
void on_received_message_default_channel(ClientState* cstate, Message* msg){
    while(cstate->nb_msgs_default_channel >= cstate->tot_msgs_default_channel){
        size_t tmp = cstate->tot_msgs_default_channel;
        cstate->tot_msgs_default_channel *= 2;
        CHKN( cstate->msgs_default_channel
                = realloc(cstate->msgs_default_channel,
                          sizeof(Message) * cstate->tot_msgs_default_channel) );
        CHKN( cstate->heights_msgs_default_channel
                = realloc(cstate->heights_msgs_default_channel,
                          sizeof(Message) * cstate->tot_msgs_default_channel) );
        //
        for(size_t i=tmp; i<cstate->tot_msgs_default_channel; i++){
            init_empty_message(&(cstate->msgs_default_channel[i]));
            cstate->heights_msgs_default_channel[i] = 0;
        }
    }
    //
    copy_message(&(cstate->msgs_default_channel[cstate->nb_msgs_default_channel]),
                 msg);
    // 
    if((size_t)cstate->disp_msgs_cursor == cstate->nb_msgs_default_channel-1){
        cstate->disp_msgs_cursor++;
    }
    cstate->nb_msgs_default_channel++;
}


/*
    ------------------------ Side Display Functions ------------------------
*/

#define MSG_HOVER_COLOR GREEN_YELLOW


// Compute the total displayed height of one message
int calc_one_msg_tot_height(Message* msg, int max_text_line_length){
    //
    int height = 3;
    int i = 0;
    //
    while((int)msg->msg_length - i > max_text_line_length){
        i+=max_text_line_length;
        height++;
    }
    if(msg->msg_length - i > 0){
        height++;
    }
    //
    return height;
}


// Compute the total displayed height of all the msgs in the default channel
void calc_msgs_tot_height(ClientState* cstate,
                          int tot_width
){
    //
    int max_text_line_length = tot_width - 3;
    //
    for(size_t j=0; j<cstate->nb_msgs_default_channel; j++){
        cstate->heights_msgs_default_channel[j]
            = calc_one_msg_tot_height(&(cstate->msgs_default_channel[j]),
                                      max_text_line_length);
    }
}


// Prints a message, visible only on a certain boundary limit,
//   and returns the y coordinate of the last message printed line
int print_message(ClientState* cstate, Message* msg,
                   int x, int y,
                   int max_x, int min_y_show, int max_y_show,
                   bool hover
){
    //
    int tot_width = max_x - x;

    //
    set_cursor_position(x, y);

    // Display the header Message borders
    if(hover){
        set_cl_fg(MSG_HOVER_COLOR);
        set_bold();
    }
    if( y >= min_y_show && y <= max_y_show )
        { print_horizontal_line('-', x, x+5, y); }
    
    if( y+1 >= min_y_show && y+1 <= max_y_show ){ 
        set_cursor_position(x, y+1);
        printf("|");
    }

    if( y+2 >= min_y_show && y+2 <= max_y_show )
        { print_horizontal_line('-', x, x+3, y+2); }
    
    if(hover)
        { reset_ansi(); }
    
    // Display the header Message
    if( y+1 >= min_y_show && y+1 <= max_y_show ){
        int pseudo_len = strlen(msg->src_pseudo);
        if(msg->msg_type == MSG_SERVER_CLIENT && pseudo_len > 0){
            if(strcmp(cstate->pseudo, msg->src_pseudo) == 0){
                // -- its a message that this client has sent --

                set_cursor_position(x+2, y+1);
                set_cl_fg(LIGHT_SALMON);
                set_bold();
                printf("You");
                reset_ansi();

            }
            else{
                // -- its a message from another client --

                set_cursor_position(x+2, y+1);

                int xplus = strlen("| From ");
                
                // Truncate the pseudo if too long
                if(pseudo_len > tot_width - xplus){
                    char tmp = msg->src_pseudo[tot_width - xplus - 2];
                    msg->src_pseudo[tot_width - xplus - 2] = '\0';
                    printf("From %s..", msg->src_pseudo);
                    msg->src_pseudo[tot_width - xplus - 2] = tmp;
                }
                else{
                    printf("From %s", msg->src_pseudo);
                }
            }

        }
    }

    // Display the message content
    int cy = y + 3;
    int i = 0;
    int max_text_line_length = tot_width - 3;
    while((int)msg->msg_length - i > max_text_line_length){

        if( cy >= min_y_show && cy <= max_y_show ){ 
            set_cursor_position(x, cy);

            // Print the msg border decoration
            if(hover){
                set_cl_fg(MSG_HOVER_COLOR);
                set_bold();
            }
            printf(">");
            if(hover)
                { reset_ansi(); }
            
            // Print the current line of the message
            int p_end = i + max_text_line_length;
            char tmp = msg->msg[p_end];
            msg->msg[p_end] = '\0';
            printf("%s", msg->msg + i);
            msg->msg[p_end] = tmp;
        }

        i += max_text_line_length;
        cy += 1;
    }
    
    // Print the last line of the message
    if(msg->msg_length - i > 0 && cy >= min_y_show && cy <= max_y_show){
        set_cursor_position(x, cy);

        // Print the msg border decoration
        if(hover){
            set_cl_fg(MSG_HOVER_COLOR);
            set_bold();
        }
        printf(">");
        if(hover)
            { reset_ansi(); }
        
        // Print the current line of the message
        printf("%s", msg->msg + i);
    }

    return cy - 1;
}


// Display the left panel: the messages of the current channel
void display_left_panel(ClientState* cstate){
    // Requirements
    int x_barriere_top = cstate->win_width - cstate->logo_main->tx - 1;
    int y_barriere_bottom = cstate->win_height - 4;
    //
    int panel_x_start = 3;
    int panel_x_end = x_barriere_top-1;
    int panel_y_start = 3;
    int panel_y_end = y_barriere_bottom - 2;
    int panel_width = panel_x_end - panel_x_start;
    int panel_height = panel_y_end - panel_y_start;
    //
    int spaces_between_msgs = 1;
    //
    int y_0 = 0;
    int id_first_msg_disp = 0;
    int id_last_msg_disp = 0;
    //
    if(cstate->type_current_dest == MSG_FLAG_DEFAULT_CHANNEL){
        if(cstate->nb_msgs_default_channel == 0){
            return;
        }
        //
        calc_msgs_tot_height(cstate, panel_width);

        // Current Message Height
        int cmh = cstate->heights_msgs_default_channel[cstate->disp_msgs_cursor];

        // Sum of the height of precedents and post messages to the focused msg 
        int sum_pre = 0;
        int sum_post = 0;
        for(int i=0; i<cstate->disp_msgs_cursor; i++){
            sum_pre += cstate->heights_msgs_default_channel[i];
        }
        for(size_t i=cstate->disp_msgs_cursor+1; i<cstate->nb_msgs_default_channel; i++){
            sum_post += cstate->heights_msgs_default_channel[i];
        }
        
        // Differents messages positions cases
        if(sum_pre + cmh < panel_height / 2){

            y_0 = 0;
            id_first_msg_disp = 0;
            int cy = 0;
            id_last_msg_disp = -1;
            //
            for(size_t i = 0; i<cstate->nb_msgs_default_channel; i++){
                //
                cy += cstate->heights_msgs_default_channel[i];
                cy += spaces_between_msgs;
                //
                if(cy > panel_width){
                    id_last_msg_disp = i;
                    break;
                }
            }
            if(id_last_msg_disp == -1){
                id_last_msg_disp = cstate->nb_msgs_default_channel-1;
            }
        }
        else if(sum_post + cmh < panel_height / 2){

            id_last_msg_disp = cstate->nb_msgs_default_channel - 1;
            id_first_msg_disp = -1;
            y_0 = panel_height;
            //
            for(int i = id_last_msg_disp; i>=0; i--){
                //
                y_0 -= cstate->heights_msgs_default_channel[i];
                y_0 -= spaces_between_msgs;
                //
                if(y_0 <= 0){
                    id_first_msg_disp = i;
                    break;
                }
            }
            if(id_first_msg_disp == -1){
                id_first_msg_disp = 0;
            }
        }
        else{

            int cy = panel_height / 2 - cmh / 2;
            // Top part
            y_0 = cy;
            id_first_msg_disp = -1;
            for(int i=cstate->disp_msgs_cursor-1; i>=0; i--){
                y_0 -= cstate->heights_msgs_default_channel[i];
                y_0 -= spaces_between_msgs;
                //
                if(y_0 <= 0){
                    id_first_msg_disp = i;
                    break;
                }
            }
            // Bottom part
            cy += cmh;
            id_last_msg_disp = -1;
            //
            for(size_t i = cstate->disp_msgs_cursor+1; i<cstate->nb_msgs_default_channel; i++){
                //
                cy += cstate->heights_msgs_default_channel[i];
                cy += spaces_between_msgs;
                //
                if(cy > panel_width){
                    id_last_msg_disp = i;
                    break;
                }
            }
            if(id_last_msg_disp == -1){
                id_last_msg_disp = cstate->nb_msgs_default_channel-1;
            }
        }

        // We now have theses three variables initialised:
        //   y_0 - id_first_msg_disp - id_last_msg_disp

        // We can now FINALLY DISPLAYS THE MESSAGES
        int cy = panel_y_start + y_0;
        for(int i=id_first_msg_disp; i<=id_last_msg_disp; i++){
            print_message(cstate,
                          &(cstate->msgs_default_channel[i]),
                          panel_x_start, cy,
                          panel_x_end,
                          panel_y_start,
                          panel_y_end,
                          i == cstate->disp_msgs_cursor);
            cy += cstate->heights_msgs_default_channel[i];
            cy += spaces_between_msgs;
        }
        //
    }
    else{
        // TODO
    }
}


// Display the top right panel: The current menu
void display_top_right_panel(ClientState* cstate){
    // Requirements
    int x_barriere_top = cstate->win_width - cstate->logo_main->tx - 1;
    //
    int panel_x_start = x_barriere_top + 2;
    int panel_x_end = cstate->win_width - 2;
    int panel_y_start = 5;
    // int panel_y_end = y_barriere_bottom - 7;
    // int panel_width = panel_x_end - panel_x_start;
    // int panel_height = panel_y_end - panel_y_start;
    //

    char titre_col[] = "Infos";
    print_centered_text(
        titre_col, strlen(titre_col), 
        panel_x_start + 1,
        panel_x_end - 1,
        panel_y_start + 1
    );


}


// Display the bottom right panel: The options of the current menu
void display_bottom_right_panel(ClientState* cstate){

    (void)cstate;

    // Requirements
    int x_barriere_top = cstate->win_width - cstate->logo_main->tx - 1;
    // int y_barriere_bottom = cstate->win_height - 4;
    //
    int panel_x_start = x_barriere_top + 1;
    int panel_x_end = cstate->win_width - 1;
    int panel_y_start = 9;
    // int panel_y_end = y_barriere_bottom - 2;
    // int panel_width = panel_x_end - panel_x_start;
    // int panel_height = panel_y_end - panel_y_start;
    //

    set_bold();
    set_cl_fg(LIGHT_SALMON);
    char Titre_Pseudo[] = "Pseudo:";
    print_centered_text(Titre_Pseudo, strlen(Titre_Pseudo), panel_x_start+1, panel_x_end-1, panel_y_start + 2);
    reset_ansi();

    print_centered_text(cstate->pseudo, strlen(cstate->pseudo), panel_x_start+1, panel_x_end-1, panel_y_start + 4);

    set_bold();
    set_cl_fg(LIGHT_SALMON);
    char Titre_Channel[] = "Current Channel:";
    print_centered_text(Titre_Channel, strlen(Titre_Channel), panel_x_start+1, panel_x_end-1, panel_y_start + 8);
    reset_ansi();   

    char channel[] = "Default Channel";
    print_centered_text(channel, strlen(channel), panel_x_start+1, panel_x_end-1, panel_y_start + 10);
}


// Display the input text with the cursor
void display_input_text(ClientState* cstate, int x, int y, int max_x){

    int ctx = max_x - x;

    // Easy case: the input length is smaller than the input container
    if (cstate->input_length <= ctx){
        set_cursor_position(x, y);
        printf("%s", cstate->input);
        cstate->cursor_x = x + cstate->input_cursor;
        cstate->cursor_y = y;
    }
    //
    else{
        int p_start;
        int p_end;
        int p_curs;

        //
        if(cstate->input_cursor <= ctx / 2){
            p_start = 0;
            p_end = ctx;
            p_curs =cstate->input_cursor;
        }
        else if(cstate->input_cursor >= cstate->input_length - ctx/2){
            p_start = cstate->input_length - ctx;
            p_end = cstate->input_length;
            p_curs = cstate->input_cursor - p_start;
        }
        else{
            p_start = cstate->input_cursor - ctx/2;
            p_end = cstate->input_length + ctx/2;
            p_curs = cstate->input_cursor - p_start;
        }

        //
        set_cursor_position(x, y);
        char tmp = cstate->input[p_end];
        cstate->input[p_end] = '\0';
        printf("%s", cstate->input + p_start);
        cstate->input[p_end] = tmp;

        cstate->cursor_x = x + p_curs;
        cstate->cursor_y = y;
    }

    // Display cursor
    if (cstate->user_focus == FOCUS_INPUT) {
        //
        set_cursor_position(cstate->cursor_x, cstate->cursor_y);
        show_cursor();
    }

}


/*
    ------------------------ Client Display Functions ------------------------
*/

// Set the focus color, considerating if hard focus or not
void set_focus_color(ClientState* cstate){
    set_bold();
    if(cstate->user_focus != FOCUS_INPUT && cstate->hard_focus){
        set_cl_fg(HARD_FOCUS_COLOR);
    }
    else{
        set_cl_fg(FOCUS_COLOR);
    }
}


// Display the Connection Window
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


// Display the Main Interface Window (when the client is connected)
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
                                   GREEN, LIGHT_GREEN, RED);

    // -- menus --
    print_horizontal_line('#', x_barriere_top+1, cstate->win_width-1, 4);
    if (cstate->user_focus == FOCUS_RIGHT_TOP_PANEL)
        { set_focus_color(cstate); }
    
    print_vertical_line('|', x_barriere_top+1, 5, 7);
    print_vertical_line('|', cstate->win_width-1, 5, 7);
    if (cstate->user_focus == FOCUS_INPUT)
        { reset_ansi(); }

    //
    display_top_right_panel(cstate);

    // Transition menus - right panel
    set_cursor_position(x_barriere_top+1, 8);
    printf("#");
    set_cursor_position(cstate->win_width-1, 8);
    printf("#");
    if(cstate->user_focus == FOCUS_RIGHT_TOP_PANEL ||
       cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL
    ){
        set_focus_color(cstate);
    }
    print_horizontal_line('-', x_barriere_top+2, cstate->win_width-2, 8);
    if(cstate->user_focus == FOCUS_RIGHT_TOP_PANEL ||
       cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL
    ){
        reset_ansi();
    }

    // Right pannel
    if(cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL)
        { set_focus_color(cstate); }

    print_horizontal_line('-', x_barriere_top+1, cstate->win_width-2, cstate->win_height-5);
    if(cstate->user_focus == FOCUS_RIGHT_BOTTOM_PANEL){
        reset_ansi();
    }

    //
    display_bottom_right_panel(cstate);

    // Messages pannel
    if(cstate->user_focus == FOCUS_LEFT_PANEL)
        { set_focus_color(cstate); }
    
    print_horizontal_line('-', 2, x_barriere_top-1, 2);
    print_horizontal_line('-', 2, x_barriere_top-1, cstate->win_height-5);
    if(cstate->user_focus == FOCUS_LEFT_PANEL){
        reset_ansi();
    }

    //
    display_left_panel(cstate);

    // -- Bottom input --
    print_horizontal_line('#', 1, cstate->win_width-2, y_barriere_bottom);
    if(cstate->user_focus == FOCUS_INPUT)
        { set_focus_color(cstate); }
    
    print_vertical_line('|', 2, cstate->win_height-3, cstate->win_height-1);
    print_vertical_line('|', cstate->win_width-2, cstate->win_height-3, cstate->win_height-1);
    print_horizontal_line('-', 2, cstate->win_width - 2, cstate->win_height-3);
    print_horizontal_line('-', 2, cstate->win_width - 2, cstate->win_height-1);
    if (cstate->user_focus == FOCUS_INPUT)
        { reset_ansi(); }

    set_cursor_position(3, cstate->win_height-2);
    printf(">");

    // at the end, we print the input and set the cursor at the good input position, if input focus
    display_input_text(cstate, 5, cstate->win_height-2, cstate->win_width - 4);
    
    force_buffer_prints();
}


// If the terminal windows is too small, display an error
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


// Main function to display the client
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


// Function that process a message that the client wants to send
void client_process_input_message(TcpConnection* con, 
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


// Main entry point for all stdin key events (raw mode)
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
    // Okay, after simplification, user_focus is always in FOCUS_INPUT
    // But I will still keep this condition
    // to remembert if I want to change that leater
    if(cstate->user_focus == FOCUS_INPUT){
        if(buffer[0] == '\x1b'){
            if(buffer[1] == SPECIAL_CHAR_ARROW){
                if(buffer[2] == ARROW_LEFT){
                    if(cstate->input_cursor > 0){
                        cstate->input_cursor--;
                    }
                }
                else if(buffer[2] == ARROW_RIGHT){
                    if(cstate->input_cursor < cstate->input_length){
                        cstate->input_cursor++;
                    }
                }
                else if(buffer[2] == ARROW_BOTTOM){
                    if((size_t)cstate->disp_msgs_cursor < cstate->nb_msgs_default_channel-1){
                        cstate->disp_msgs_cursor++;
                    }
                }
                else if(buffer[2] == ARROW_TOP){
                    if(cstate->disp_msgs_cursor > 0){
                        cstate->disp_msgs_cursor--;
                    }
                }
            } else if(buffer[1] == SPECIAL_CHAR_KEYS){
                if(buffer[2] == K_ENTER){
                    client_process_input_message(con, cstate);
                    cstate->input_length = 0;
                    cstate->input_cursor = 0;
                    cstate->input[0] = '\0';
                }
                else if(buffer[2] == K_TABULATION && cstate->connected){
                    // cstate->user_focus = FOCUS_LEFT_PANEL;
                    // cstate->hard_focus = false;
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

    //
    display_client(cstate);
}


// Main Entry point for all the messages received from the buffer
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

            // TODO : mettre ca dans une case d'erreur
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
            cstate->input_length = 0;
            cstate->input_cursor = 0;
            cstate->input[0] = '\0';
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
                if(msg->dst_flag == MSG_FLAG_DEFAULT_CHANNEL){
                    on_received_message_default_channel(cstate, msg);
                    display_client(cstate);
                }
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
                // Update the number of times it has been retransmissed
                cstate->msg_waiting_ack[msg->msg_id].nb_retransmission
                                                    = msg->nb_retransmission;

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



void send_disconnection_message(TcpConnection* con, ClientState* cstate){
    // Send connection request to the server
    init_empty_message(&(cstate->msg_waiting_ack[0]));
    cstate->msg_waiting_ack[0].msg_type = MSG_CLIENT_DISCONNECT;
    cstate->msg_waiting_ack[0].msg_id = 0;
    strcpy(cstate->msg_waiting_ack[0].src_pseudo, cstate->pseudo);
    cstate->nb_msg_waiting_ack += 1;
    //
    tcp_connection_message_send(con, con->sockfd,
                                &(cstate->msg_waiting_ack[0]));
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

    // Init default channel messages received
    cstate->tot_msgs_default_channel = 10;
    cstate->nb_msgs_default_channel = 0;
    CHKN( cstate->msgs_default_channel
                = calloc(cstate->tot_msgs_default_channel, sizeof(Message)) );
    CHKN( cstate->heights_msgs_default_channel
                = calloc(cstate->tot_msgs_default_channel, sizeof(int)) );
    //
    for (size_t i=0; i<cstate->tot_msgs_default_channel; i++)
        { init_empty_message(&(cstate->msgs_default_channel[i])); }


    // Init user focus
    cstate->user_focus = FOCUS_INPUT;

    // Init cursor for the message windows display
    cstate->disp_msgs_cursor = 0;

    // Init the Input
    CHKN( memset(cstate->input, '\0', MAX_MSG_LENGTH) );
    cstate->input_length = 0;
    cstate->input_cursor = 0;
    cstate->cursor_x = 0;
    cstate->cursor_y = 0;

    // Init win size
    cstate->win_height = 0;
    cstate->win_height = 0;

    // Init logos
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


// Main entry point of the client
int main(int argc, char* argv[]) {
    
    fprintf(stderr, "testt\n");

    // Base variables, containers for ClientState and TcpConnection
    ClientState cstate;
    TcpConnection con;

    // Check arguments
    if (argc != 3) {
        printf("Usage: %s ip_proxy port_proxy\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Get arguments
    char* ip_proxy = argv[1];
    int port_proxy = atoi(argv[2]);

    // Check arguments
    if (port_proxy < 5000 || port_proxy > 65000) {
        fprintf(stderr, "Bad value of port_proxy : %d !\n", port_proxy);
        exit(EXIT_FAILURE);
    }


    // Enabling raw terminal mode to get individually each characters
    enableRawMode(&orig_termios);

    // Launching the client and the connection
    init_cstate(&cstate);
    tcp_connection_client_init(&con, ip_proxy, port_proxy, -1);
    con.ansi_stdin = true;

    // First display of the client
    display_client(&cstate);

    // Client mainloop (with polling)
    tcp_connection_mainloop(&con, on_msg_client, &cstate,
                            on_stdin_client, &cstate);


    // Closing the connection and the client
    //   and freeing all allocated variables
    tcp_connection_close(&con);
    free_cstate(&cstate);

    return 0;
}
