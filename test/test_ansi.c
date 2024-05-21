
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <time.h>

#include "lib_ansi.h"

void setUp(void) {}
void tearDown(void) {}

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


int main() {
    int width, height;

    AsciiArt* logo = load_ascii_art("res/test2.txt");

    get_terminal_size(&width, &height);

    printf("Terminal width: %d, Terminal height: %d\n", width, height);

    set_screen_border(width, height);


    //
    while(1){
        get_terminal_size(&width, &height);
        set_screen_border(width, height);
        
        print_ascii_art_with_gradients(width/3, height/3, logo,
                                       FUCHSIA, CYAN, GREEN);
        
        sleep(1);
    }

    free_ascii_art(logo);
    return 0;
}
