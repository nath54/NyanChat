
/*
    ------------------------ Includes ------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>

#include "lib_ansi.h"
#include "lib_chks.h"


/*
    ------------------------ Global Variables ------------------------
*/

// To save the original state of the terminal to restore it at the end
termios_t orig_termios;



/*
    ------------------------ functions ------------------------ 
*/


// Print the input got char
void print_key(char c){
    if (iscntrl(c)) {
        printf("%d", c);
    } else {
        printf("%d ('%c')", c, c);
    }
}

/*
    __________ Input Related functions __________
*/

char appReadKey() {
    int nread;
    char c;
    while (1){
        CHKERRNO( nread = read(STDIN_FILENO, &c, 1), EAGAIN );
    }
    return c;
}


void appProcessKeypress() {
    char c = appReadKey();
    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
        
        default:
            break;
    }
}

/*
    __________ Idk what this is __________ 
*/

void setUp(void) {}
void tearDown(void) {}


/*
    ------------------------ main ------------------------ 
*/

int main() {

    // Enabling raw terminal mode to get individually each characters
    enableRawMode(&orig_termios);

    // To store at each moment the width and the height of the terminal
    int width, height;

    // Loading the logo to display in the ui
    AsciiArt* logo = load_ascii_art("res/test2.txt");

    // Get the terminal size
    get_terminal_size(&width, &height);

    // Program mainloop
    while (1) {
        // Reading the terminal input
        appProcessKeypress();

        // Get the terminal size
        get_terminal_size(&width, &height);

        // Displaying things on the screen
        set_screen_border(width, height - 4);
        print_ascii_art_with_gradients(width/3, height/3, logo,
                                       FUCHSIA, CYAN, GREEN);
        
    }

    // Freeing resources at the end
    free_ascii_art(logo);

    // Exit the program, exit is called on main return
    //  so the resetTerminalMode is called here
    return 0;
}
