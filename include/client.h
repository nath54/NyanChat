#pragma once

#include <stdbool.h>
#include "lib_client_server.h"
#include "lib_ansi.h"

typedef struct {
    char pseudo[MAX_NAME_LENGTH];

    // Liste of the lasts messages
    Message msgs[NB_MAX_MESSAGES];

    // Number of messages in this channel
    int nb_msgs;

    // Indicates that NB_MAX_MESSAGES messages have been exceeded in the chat room,
    //  therefore the periodicity of the msgs table is activated
	bool msgs_boucle;

} ConClient;


#define FOCUS_INPUT 0
#define FOCUS_LEFT_PANEL 1
#define FOCUS_RIGHT_TOP_PANEL 2
#define FOCUS_RIGHT_BOTTOM_PANEL 3
#define FOCUS_COLOR INDIGO
#define HARD_FOCUS_COLOR ORANGE

typedef struct {
    // Pseudo
    char pseudo[MAX_NAME_LENGTH];

    //
    bool connected;
    bool waiting_pseudo_confirmation;

    //
    char* connection_code;

    // Current place type to send messages
    // 0=default channel, 1=private channel, 2=private discussion
    int type_current_dest;
    // Current place name, pseudo or channel name
    char destination[MAX_NAME_LENGTH];

    // DynArray of sent messages that are waiting for an acknowledgement
    Message* msg_waiting_ack;
    size_t tot_msg_waiting_ack;
    size_t nb_msg_waiting_ack;

    // DynArray of connected clients
    ConClient* connected_clients;
    size_t tot_connected_clients;
    size_t nb_connected_clients;
    int cursor_connected_clients;

    // DynArray of channels
    Channel* channels;
    size_t tot_channels;
    size_t nb_channels;

    // DynArray of messages received from default channel
    Message* msgs_default_channel;
    size_t tot_msgs_default_channel;
    size_t nb_msgs_default_channel;

    // For knowing where the user focus is
    int user_focus;
    // Press escape to quit hard focus mode and navigate the panels
    bool hard_focus;

    // For displaying messages of a channel
    int disp_msgs_cursor;

    //
    char input[MAX_MSG_LENGTH];    
    int input_length;
    int input_cursor;

    int cursor_x;
    int cursor_y;

    // For knowing the window size
    int win_width;
    int win_height;

    // logo
    AsciiArt* logo_connection;
    AsciiArt* logo_main;

} ClientState;


#define MIN_TERMINAL_WIDTH 95
#define MIN_TERMINAL_HEIGHT 55
