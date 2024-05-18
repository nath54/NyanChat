
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <time.h>

// Function to get the terminal width and height
void get_terminal_size(int *width, int *height) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *width = w.ws_col;
    *height = w.ws_row;
}


// Function to get the cursor position
void get_cursor_position(int *row, int *col) {
    printf("\033[6n");  // ANSI escape code to query cursor position
    scanf("\033[%d;%dR", row, col);  // Parse the response
}

// Function to set the cursor position
void set_cursor_position(int row, int col) {
    printf("\033[%d;%dH", row, col);  // ANSI escape code to set cursor position
}


int main() {
    int width, height;

    get_terminal_size(&width, &height);

    double w1 = width / 4;
    double w2 = width * 3 / 4;
    int h1 = height / 2;

    double cs;
    int cw;


    printf("Terminal width: %d, Terminal height: %d\n", width, height);

    //
    while(1){
        get_terminal_size(&width, &height);

        cs = sin(time(NULL));
        cw = w1 + cs * (w2 - w1);

        // printf("%lf, %d\n", cs, cw);

        set_cursor_position(h1, cw);
    }
    return 0;
}
