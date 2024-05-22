
/*
    ------------------------ Includes ------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>

#include "lib_ansi.h"
#include "useful_lib.h"
#include "lib_chks.h"



/*
    ------------------------ Color Related Functions ------------------------
*/


double my_clamp(double val, double vmin, double vmax){
    if(val < vmin){
        return vmin;
    }
    if(val > vmax){
        return vmax;
    }
    return val;
}

// Create a Color struct from r, g, b
Color cl(double r, double g, double b){
    return (Color){r, g, b};
}

// Clamp a color values between 0 and 255
Color cl_clamp(Color c){
    return (Color){
        my_clamp(c.r, 0, 255),
        my_clamp(c.g, 0, 255),
        my_clamp(c.b, 0, 255)
    };
}

// Add two colors together
Color cl_add(Color c1, Color c2){
    return (Color){c1.r + c2.r, c1.g + c2.g, c1.b + c2.b};
}

// Substract two colors
Color cl_sub(Color c1, Color c2){
    return (Color){c1.r - c2.r, c1.g - c2.g, c1.b - c2.b};
}

// Divide a color by a value
Color cl_divide(Color c1, double d){
    return (Color){c1.r / d, c1.g / d, c1.b / d};
}

// Multiply a color by a value
Color cl_mult(Color c1, double v){
    return (Color){c1.r * v, c1.g * v, c1.b * v};
}

// Set the terminal font color to the color c
void set_cl_fg(Color c){
    printf("\033[38;2;%d;%d;%dm", (int)c.r, (int)c.g, (int)c.b);
}

// Set the terminal background color to the color c
void set_cl_bg(Color c){
    printf("\033[48;2;%d;%d;%dm", (int)c.r, (int)c.g, (int)c.b);
}



// Rainbow colors
#define NB_RAINBOW_COLORS 7
static Color rainbow_colors[NB_RAINBOW_COLORS] = {
    RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET
};


// Print a text with a rainbow effect
void print_rainbow(char* txt){
    size_t len = strlen(txt);
    int c=0;
    int croissant=1;
    for(size_t i=0; i<len; i++){
        set_cl_fg(rainbow_colors[c]);
        printf("%c", txt[i]);
        //
        if(croissant){
            c += 1;
            if(c == NB_RAINBOW_COLORS){
                c = NB_RAINBOW_COLORS - 2;
                croissant = 0;
            }
        }
        else{
            c -= 1;
            if(c == -1){
                c = 1;
                croissant = 1;
            }
        }
    }
    reset_ansi();
}



/*
    ------------------------ Other ANSI Code functions ------------------------
*/


// Set the terminal font to bold
void set_bold(){
    printf("\033[1m");
}

// Unset the terminal font to bold
void unset_bold(){
    printf("\033[21m");
}

// Remove all the active ANSI effects for the next prints
void reset_ansi(){
    printf("\033[m");
}



/*
    ------------------------ Console Output Functions ------------------------
*/


// Clean the terminal screen
void clean_terminal(){
    write(STDOUT_FILENO, "\x1b[2J", 4);
}


/*
    ------------------------ Terminal Functions ------------------------
*/



/*
    __________ Terminal Settings related functions __________ 
*/


// Reset the terminal to the state it was at before enabling the Raw Mode
void resetTerminalMode(int ev, void* args) {
    (void)ev;
    termios_t* orig_termios = args;
    CHK( tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios) );
        
}

// Enabling Raw Mode : allow to get all the key presses one by one
void enableRawMode(termios_t* orig_termios) {
    // Saving the original state of the terminal
    CHK( tcgetattr(STDIN_FILENO, orig_termios) );

    // Reset the terminal when exit is called
    on_exit(resetTerminalMode, (void*)orig_termios);

    // Copying the terminal state to modify it
    struct termios raw = *orig_termios;

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


/*
    __________ Other Terminal related functions __________ 
*/



// Function to get the terminal width and height
void get_terminal_size(int *width, int *height) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *width = w.ws_col;
    *height = w.ws_row;
}

// Function to get the cursor position
//  /!\\ Warning /!\\ : (x, y) parameter order
void get_cursor_position(int *col, int *row) {
    printf("\033[6n");  // ANSI escape code to query cursor position
    scanf("\033[%d;%dR", row, col);  // Parse the response
}

// Function to set the cursor position
//  /!\\ Warning /!\\ : (x, y) parameter order
void set_cursor_position(int col, int row) {
    printf("\033[%d;%dH", row, col);  // ANSI escape code to set cursor position
}



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
    ------------------------ Ascii Art Functions ------------------------
*/



// Loading an ascii art structure
AsciiArt* load_ascii_art(char* file_path){
    //
    AsciiArt* art = calloc(1, sizeof(AsciiArt));
    //
    FILE* f = fopen(file_path, "r");
    if (f == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    //
    get_file_stats(f, &(art->ty), &(art->tx));
    art->tx++;
    art->art = calloc(art->ty, sizeof(char*));
    //
    if (art->art == NULL) {
        perror("Error allocating memory");
        fclose(f);
        exit(EXIT_FAILURE);
    }
    //
    for(int l=0; l<art->ty; l++){
        art->art[l] = calloc(art->tx, sizeof(char));
        //
        if (art->art[l] == NULL) {
            perror("Error allocating memory");
            fclose(f);
            exit(EXIT_FAILURE);
        }
        //
        if (read_file_line(f, art->art[l], art->tx) == -1) {
            break;  // Exit loop on error or EOF
        }
        art->art[l][art->tx-1] = '\0';
    }
    //
    fclose(f);
    
    //
    return art;
}


// Freeing an ascii art structure
void free_ascii_art(AsciiArt* art){
    for(int l=0; l<art->ty; l++){
        free(art->art[l]);
    }
    free(art->art);
    free(art);
}


// Displaying an ascii art at the position x, y
void print_ascii_art(int x, int y, AsciiArt* art){
    for(int l=0; l<art->ty; l++){
        set_cursor_position(x, y+l);
        // art[l][tx - 1] = '\0';
        // printf("%s", art[l]);
        for(int c=0; c<art->tx; c++){
            //
            if(art->art[l][c] == '\0'){
                break;
            }
            //
            printf("%c", art->art[l][c]);
        }
    }
}


// Displaying a colored ascii art at the position x, y
void print_ascii_art_with_colors(int x, int y, ColoredAsciiArt* art,
                                               ColorPalette palette)
{
    for(int l=0; l<art->ty; l++){
        set_cursor_position(x, y+l);
        for(int c=0; c<art->tx; c++){
            //
            if(art->art[l][c] == '\0'){
                break;
            }
            //
            int id_cl = art->colors[l][c];
            if(id_cl < 0 || id_cl >= palette.nb_colors){
                fprintf(stderr, "Color Palette error, unknown color : %d\n",
                                                                      id_cl);
                exit(EXIT_FAILURE);
            }
            set_cl_fg(palette.colors[id_cl]);
            printf("%c", art->art[l][c]);
        }
    }
}


// Displaying an ascii art at the position x, y with gradient colors
void print_ascii_art_with_gradients(int x, int y,
                                    AsciiArt* art,
                                    Color cl_top_left,
                                    Color cl_bottom_left,
                                    Color cl_top_right)
{
    Color cl_delta_x = cl_divide(cl_sub(cl_top_right, cl_top_left), art->tx);
    Color cl_delta_y = cl_divide(cl_sub(cl_bottom_left, cl_top_left), art->ty);
    
    for(int l=0; l<art->ty; l++){
        Color cl = cl_add(cl_top_left, cl_mult(cl_delta_y, l));
        set_cursor_position(x, y+l);
        for(int c=0; c<art->tx; c++){
            //
            if(art->art[l][c] == '\0'){
                break;
            }
            //
            set_cl_fg(cl_clamp(cl));
            printf("%c", art->art[l][c]);
            cl = cl_add(cl, cl_delta_x);
        }
    }
}
