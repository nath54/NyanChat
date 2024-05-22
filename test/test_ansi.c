
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
    ------------------------ Defines ------------------------
*/

#define CTRL_KEY(k) ((k) & 0x1f)



/*
    ------------------------ Global Variables ------------------------
*/


// To save the original state of the terminal to restore it at the end
struct termios orig_termios;



/*
    ------------------------ functions ------------------------ 
*/


/*
    __________ Terminal Settings related functions __________ 
*/


// Reset the terminal to the state it was at before enabling the Raw Mode
void resetTerminalMode() {
    CHK( tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) );
        
}


// Enabling Raw Mode : allow to get all the key presses one by one
void enableRawMode() {
    // Saving the original state of the terminal
    CHK( tcgetattr(STDIN_FILENO, &orig_termios) );

    // Reset the terminal when exit is called
    atexit(resetTerminalMode);

    // Copying the terminal state to modify it
    struct termios raw = orig_termios;

    /* Turning off flags:
        - ECHO: feature causes each key you type to be printed to the terminal,
                so you can see what you’re typing.
                This is useful in canonical mode,
                but really gets in the way
                when we are trying to carefully render a user interface
                in raw mode. 
        - ICANON: (reading line-by-line) so now we can read byte-by-byte
        - ISIG: disabling the SIGINT and SIGTSTP signals
        - IXON: disabling the Ctrl-S and Ctrl-Q signals
        - IEXTEN: disabling the Ctrl-V signal
        - ICRNL: fixing Ctrl-M signal
        - OPOST: turning off ouput processing (\r\n)
        - BRKINT: When BRKINT is turned on,
                    a break conditionwill cause a SIGINT signal
                    to be sent to the program, like pressing Ctrl-C.
        - INPCK: enables parity checking,
                   which doesn’t seem to apply to modern terminal emulators.
        - ISTRIP: causes the 8th bit of each input byte to be stripped,
                    meaning it will set it to 0.
                    This is probably already turned off.
        - CS8: is not a flag, it is a bit mask with multiple bits,
               which we set using the bitwise-OR (|) operator
               unlike all the flags we are turning off.
               It sets the character size (CS) to 8 bits per byte.
    */ 
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /*
    // Timeout for read()

    The VMIN value sets the minimum number of bytes of input needed before
    read() can return. We set it to 0 so that read() returns as soon as there
    is any input to be read. The VTIME value sets the maximum amount of time
    to wait before read() returns. It is in tenths of a second, so we set it
    to 1/10 of a second, or 100 milliseconds. If read() times out, it will
    return 0, which makes sense because its usual return value is the number
    of bytes read.
    */
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;


    // Set the new terminal state
    CHK( tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) );
}



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
    __________ Display related functions __________ 
*/


// Displaying screen 
void set_screen_border(int width, int height){
    // Cleaning screen
    printf("\033c");

    // Top line
    set_cursor_position(0, 0);
    printf("#");
    for(int i=1; i<width-1; i++){
        printf("=");
    }
    printf("#\n");
    // Side borders
    for(int j=2; j<height; j++){
        printf("||");
        set_cursor_position(width-1, j);
        printf("||\n");
    }
    // Bottom line
    printf("#");
    for(int i=1; i<width-1; i++){
        printf("=");
    }
    printf("#");
}



/*
    ------------------------ main ------------------------ 
*/

int main() {
    // Enabling raw terminal mode to get individually each characters
    enableRawMode();

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
