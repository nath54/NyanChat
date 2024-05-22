#pragma once

/*
    ------------------------ Includes ------------------------
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>

#include "lib_ansi_colors.h"



/*
    ------------------------ Defines ------------------------
*/

#define CTRL_KEY(k) ((k) & 0x1f)

typedef struct termios termios_t;




/*
    ------------------------ Structs ------------------------
*/


typedef struct{
    char** art;
    int tx;
    int ty;
} AsciiArt;

typedef struct{
    Color* colors;
    int nb_colors;
} ColorPalette;

typedef struct{
    char** art;
    int** colors;
    int tx;
    int ty;
} ColoredAsciiArt;



/*
    ------------------------ Color Related Functions ------------------------
*/


// Create a Color struct from r, g, b
Color cl(double r, double g, double b);

// Clamp a color values between 0 and 255
Color cl_clamp(Color c);

// Add two colors together
Color cl_add(Color c1, Color c2);

// Substract two colors
Color cl_sub(Color c1, Color c2);

// Divide a color by a value
Color cl_divide(Color c1, double d);

// Multiply a color by a value
Color cl_mult(Color c1, double v);

// Set the terminal font color to the color c
void set_cl_fg(Color c);

// Set the terminal background color to the color c
void set_cl_bg(Color c);

// Print a text with a rainbow effect
void print_rainbow(char* txt);


/*
    ------------------------ Other ANSI Code functions ------------------------
*/

// Set the terminal font to bold
void set_bold();

// Unset the terminal font to bold
void unset_bold();

// Remove all the active ANSI effects for the next prints
void reset_ansi();



/*
    ------------------------ Terminal Functions ------------------------
*/


/*
    __________ Terminal Settings related functions __________ 
*/


// Reset the terminal to the state it was at before enabling the Raw Mode
//   args is in fact termios_t*
void resetTerminalMode(int ev, void* args);

// Enabling Raw Mode : allow to get all the key presses one by one
void enableRawMode(termios_t* orig_termios);


/*
    __________ Other Terminal related functions __________ 
*/


// Function to get the terminal width and height
void get_terminal_size(int *width, int *height);

// Function to get the cursor position
void get_cursor_position(int *row, int *col);

// Function to set the cursor position
void set_cursor_position(int row, int col);

// Clean the terminal screen
void clean_terminal();


// Function to print a vertical line of a character
void print_horizontal_line(char c, int x_start, int x_end, int y);

// Function that print an horizontal line of a character
void print_vertical_line(char c, int x, int y_start, int y_end);

// Force the buffer of printfs to be send to stdout
void force_buffer_prints();

// Enable cursor display
void show_cursor();

// Disable cursor display
void hide_cursor();

/*
    __________ Display related functions __________ 
*/


// Displaying screen 
void set_screen_border(int width, int height);



/*
    ------------------------ Ascii Art Functions ------------------------
*/


// Loading an ascii art structure
AsciiArt* load_ascii_art(char* file_path);

// Freeing an ascii art structure
void free_ascii_art(AsciiArt* art);

// Displaying an ascii art at the position x, y
void print_ascii_art(int x, int y, AsciiArt* art);

// Displaying a colored ascii art at the position x, y
void print_ascii_art_with_colors(int x, int y, ColoredAsciiArt* art,
                                               ColorPalette palette);

// Displaying an ascii art at the position x, y with gradient colors
void print_ascii_art_with_gradients(int x, int y,
                                    AsciiArt* art,
                                    Color cl_top_left,
                                    Color cl_bottom_left,
                                    Color cl_top_right);

