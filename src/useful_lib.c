#include "useful_lib.h"

#include "lib_chks.h"

// Generate a random code
char* generate_random_code(uint32 code_length){
    //
    char* code;
    CHKN( code = calloc(code_length, sizeof(char)) );
    //
    for(uint32 i=0; i<code_length - 1; i++){
        code[i] = 32 + rand() % 82;
    }
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

