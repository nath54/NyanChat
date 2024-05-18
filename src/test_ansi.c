
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

// Function to get the terminal width and height
void get_terminal_size(int *width, int *height) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *width = w.ws_col;
    *height = w.ws_row;
}

int main() {
    int width, height;
    get_terminal_size(&width, &height);
    printf("Terminal width: %d, Terminal height: %d\n", width, height);
    return 0;
}
