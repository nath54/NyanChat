#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>

#include "../include/lib_ansi_colors.h"

// Color Structure
typedef struct{
    int r;
    int g;
    int b;
} Color;

// Create a Color struct from r, g, b
Color cl(int r, int g, int b);

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
