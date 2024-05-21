#pragma once

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/random.h>

#define OP_FAILURE -1
#define OP_SUCCESS 0


typedef u_int32_t uint32;

/*
Here you can find all the little functions that can be useful,
but where you don't know where to put.
*/

// Generate a random code
char* generate_random_code(uint32 code_length);


/**
 * Reads a file and stores its content into a char array.
 * 
 * @param file_path The file containing the file to read.
 * @param content A pointer to hold the content. The caller must free this buffer.
 * @param len A pointer to hold the length of the content.
 * 
 * @return OP_SUCCESS on success, OP_FAILURE on failure.
 */
int read_file(const char* file_path, char** content, size_t* len);
