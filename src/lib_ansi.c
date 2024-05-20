#include "lib_ansi.h"

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


//
void get_file_stats(FILE *f, int *num_lines, int *max_length) {
  int current_length;
  char buffer[BUFSIZ];

  *num_lines = 0;
  *max_length = 0;

  // Read the file line by line
  while (fgets(buffer, sizeof(buffer), f) != NULL) {
    (*num_lines)++;  // Increment line count

    // Remove trailing newline (if present)
    current_length = strlen(buffer);
    if (buffer[current_length - 1] == '\n') {
      current_length--;
    }

    // Update max length if necessary
    if (current_length > *max_length) {
      *max_length = current_length;
    }
  }

  // Reset the file pointer to the beginning
  rewind(f);
}


//
int read_file_line(FILE *f, char *line, int max_length) {
    // Read a line from the file
    if (fgets(line, max_length + 1, f) == NULL) {
        return -1;  // Indicate end of file or error
    }

    // Remove trailing newline (if present)
    int length = strlen(line);
    if (line[length - 1] == '\n') {
        line[length - 1] = '\0';  // Replace newline with null terminator
    }

    return 0;  // Success
}


//
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


//
void free_ascii_art(AsciiArt* art){
    for(int l=0; l<art->ty; l++){
        free(art->art[l]);
    }
    free(art->art);
    free(art);
}


// 
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


// 
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


// 
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
