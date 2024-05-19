
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <time.h>

#include "../include/lib_ansi.h"

void set_screen_border(int width, int height){
    // Cleaning screen
    printf("\033c");

    // Ligne du dessus
    set_cursor_position(0, 0);
    printf("#");
    for(int i=1; i<width-1; i++){
        printf("=");
    }
    printf("#\n");
    // Bordures sur le côté
    for(int j=2; j<height; j++){
        printf("||");
        set_cursor_position(width-1, j);
        printf("||\n");
    }
    //
    printf("#");
    for(int i=1; i<width-1; i++){
        printf("=");
    }
    printf("#");
}

void set_logo(int width, int height){

}


int main() {
    int width, height;

    get_terminal_size(&width, &height);

    printf("Terminal width: %d, Terminal height: %d\n", width, height);

    set_screen_border(width, height);


    //
    while(1){
        get_terminal_size(&width, &height);
        set_screen_border(width, height);
        
        set_logo(width, height);
    }
    return 0;
}
