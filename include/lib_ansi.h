#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>

#include "../include/lib_ansi_colors.h"

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

// Set the terminal font to bold
void set_bold();

// Unset the terminal font to bold
void unset_bold();

// Remove all the active ANSI effects for the next prints
void reset_ansi();

// Function to get the terminal width and height
void get_terminal_size(int *width, int *height);

// Function to get the cursor position
void get_cursor_position(int *row, int *col);

// Function to set the cursor position
void set_cursor_position(int row, int col);


// Rainbow colors
#define NB_RAINBOW_COLORS 7
Color rainbow_colors[NB_RAINBOW_COLORS] = {
    RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET
};

// Print a text with a rainbow effect
void print_rainbow(char* txt);

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


