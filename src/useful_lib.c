#include <string.h>

#include "useful_lib.h"
#include "lib_chks.h"


// Return random number between 0 and max_val-1
int randint(int max_val)
    { return rand() % max_val; }



// Generate a random code
char* generate_random_code(uint32 code_length)
{
    //
    char* code;
    CHKN( code = calloc(code_length, sizeof(char)) );
    //
    for(uint32 i=0; i<code_length - 1; i++)
        { code[i] = 32 + rand() % 82; }
    //
    code[code_length - 1] = '\0';
    //
    return code;
}


/**
 * Reads a file and stores its content into a char array.
 * 
 * @param file_path The file containing the file to read.
 * @param content A pointer to hold the content. The caller must free this buffer.
 * @param len A pointer to hold the length of the content.
 * 
 * @return OP_SUCCESS on success, OP_FAILURE on failure.
 */
int read_file(const char* file_path, char** content, size_t* len)
{
    FILE* file = NULL;
    char* buffer = NULL;
    int32_t length = 0;
    int ret = OP_FAILURE;

    // Open the file
    file = fopen(file_path, "rb");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        return 0;
    }

    // Seek to the end of the file to determine its length
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error seeking to end of file\n");
        goto cleanup;
    }

    // Get the file length
    length = ftell(file);
    if (length == -1) {
        fprintf(stderr, "Error determining file length\n");
        goto cleanup;
    }

    // Return to the beginning of the file
    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to beginning of file\n");
        goto cleanup;
    }

    // Allocate memory for the content
    buffer = (char*)malloc(length);
    if (!buffer) {
        fprintf(stderr, "Error allocating memory\n");
        goto cleanup;
    }

    // Read the file content into the buffer
    if (fread(buffer, 1, length, file) != (size_t)length) {
        fprintf(stderr, "Error reading file content\n");
        free(buffer);
        goto cleanup;
    }

    // Set the output parameters
    *content = buffer;
    *len = length;
    ret = OP_SUCCESS;

cleanup:
    if (file) fclose(file);

    return ret;
}




// Get the number of lines of a file and get the max line length of a file
void get_file_stats(FILE *f, int *num_lines, int *max_length)
{
    int current_length;
    char buffer[BUFSIZ];

    *num_lines = 0;
    *max_length = 0;

    // Read the file line by line
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        (*num_lines)++;  // Increment line count

        // Remove trailing newline (if present)
        current_length = strlen(buffer);
        if (buffer[current_length - 1] == '\n')
            { current_length--; }

        // Update max length if necessary
        if (current_length > *max_length)
            { *max_length = current_length; }
    }

    // Reset the file pointer to the beginning
    rewind(f);
}


// Read a line of a file, and getting it in the pre-allocated line char array
int read_file_line(FILE *f, char *line, int max_length)
{
    // Read a line from the file
    if (fgets(line, max_length + 1, f) == NULL)
        { return -1; }  // Indicate end of file or error

    // Remove trailing newline (if present)
    int length = strlen(line);
    if (line[length - 1] == '\n')
        { line[length - 1] = '\0'; }  // Replace newline with null terminator

    return 0;  // Success
}