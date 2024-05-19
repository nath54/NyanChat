#include "../include/lib_ansi.h"

// Create a Color struct from r, g, b
Color cl(int r, int g, int b){
    return (Color){r, g, b};
}

// Set the terminal font color to the color c
void set_cl_fg(Color c){
    printf("\033[38;2;%d;%d;%dm", c.r, c.g, c.b);
}

// Set the terminal background color to the color c
void set_cl_bg(Color c){
    printf("\033[48;2;%d;%d;%dm", c.r, c.g, c.b);
}

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