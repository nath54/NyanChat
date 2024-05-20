#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "rsa.h"


/**
 * Generates an RSA key pair and saves the private and public keys
 * to the specified files.
 * 
 * @param private_key_file The file to save the private key.
 * @param public_key_file The file to save the public key.
 * @param key_size The size of the RSA key in bits.
 * 
 * @return RSA_OP_SUCCESS on success, RSA_OP_FAILURE on failure.
 * 
 * Example use:

    > generate_keypair("private.pem", "public.pem", 2048);

 */
int generate_keypair(const char* private_key_file, const char* public_key_file,
                     int key_size)
{
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    FILE *private_fp = NULL, *public_fp = NULL;
    int ret = RSA_OP_FAILURE;

    // Create context for key generation
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        fprintf(stderr, "Error creating context\n");
        goto cleanup;
    }

    // Initialize key generation
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        fprintf(stderr, "Error initializing key generation\n");
        goto cleanup;
    }

    // Set RSA key size
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_size) <= 0) {
        fprintf(stderr, "Error setting RSA key size\n");
        goto cleanup;
    }

    // Generate the key pair
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "Error generating key pair\n");
        goto cleanup;
    }

    // Open files for writing keys
    private_fp = fopen(private_key_file, "wb");
    public_fp = fopen(public_key_file, "wb");
    if (!private_fp || !public_fp) {
        fprintf(stderr, "Error opening files for writing keys\n");
        goto cleanup;
    }

    // Write private key in PEM format
    if (!PEM_write_PrivateKey(private_fp, pkey, NULL, NULL, 0, NULL, NULL)) {
        fprintf(stderr, "Error writing private key\n");
        goto cleanup;
    }

    // Write public key in PEM format
    if (!PEM_write_PUBKEY(public_fp, pkey)) {
        fprintf(stderr, "Error writing public key\n");
        goto cleanup;
    }

    ret = RSA_OP_SUCCESS;

cleanup:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pkey) EVP_PKEY_free(pkey);
    if (private_fp) fclose(private_fp);
    if (public_fp) fclose(public_fp);

    return ret;
}


/**
 * Encrypts a message using a public key from the specified file.
 * 
 * @param message The message to encrypt.
 * @param message_len The length of the message.
 * @param public_key_file The file containing the public key in PEM format.
 * @param encrypted_message A pointer to hold the encrypted message.
 *                              The caller must free this buffer.
 * @param encrypted_len A pointer to hold the length of the encrypted message.
 * 
 * @return RSA_OP_SUCCESS on success, RSA_OP_FAILURE on failure.
 */
int encrypt_message(const char* message, size_t message_len, 
                    const char* public_key_file,
                    unsigned char** encrypted_message,
                    size_t* encrypted_len)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    FILE *public_fp = NULL;
    unsigned char *encrypted = NULL;
    size_t outlen;
    int ret = RSA_OP_FAILURE;

    // Open the public key file
    public_fp = fopen(public_key_file, "rb");
    if (!public_fp) {
        fprintf(stderr, "Error opening public key file\n");
        goto cleanup;
    }

    // Read the public key
    pkey = PEM_read_PUBKEY(public_fp, NULL, NULL, NULL);
    if (!pkey) {
        fprintf(stderr, "Error reading public key\n");
        goto cleanup;
    }

    // Create context for encryption
    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) {
        fprintf(stderr, "Error creating context\n");
        goto cleanup;
    }

    // Initialize encryption
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        fprintf(stderr, "Error initializing encryption\n");
        goto cleanup;
    }

    // Set padding
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        fprintf(stderr, "Error setting padding\n");
        goto cleanup;
    }

    // Determine buffer length
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, (unsigned char*)message, message_len) <= 0) {
        fprintf(stderr, "Error determining buffer length\n");
        goto cleanup;
    }

    // Allocate buffer
    encrypted = (unsigned char*)malloc(outlen);
    if (!encrypted) {
        fprintf(stderr, "Error allocating memory\n");
        goto cleanup;
    }

    // Perform encryption
    if (EVP_PKEY_encrypt(ctx, encrypted, &outlen, (unsigned char*)message, message_len) <= 0) {
        fprintf(stderr, "Error encrypting message\n");
        free(encrypted);
        goto cleanup;
    }

    *encrypted_message = encrypted;
    *encrypted_len = outlen;
    ret = RSA_OP_SUCCESS;

cleanup:
    if (public_fp) fclose(public_fp);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pkey) EVP_PKEY_free(pkey);
    
    return ret;
}


/**
 * Decrypts a message using a private key from the specified file.
 * 
 * @param encrypted_message The encrypted message to decrypt.
 * @param encrypted_len The length of the encrypted message.
 * @param private_key_file The file containing the private key in PEM format.
 * @param decrypted_message A pointer to hold the decrypted message.
 *                              The caller must free this buffer.
 * @param decrypted_len A pointer to hold the length of the decrypted message.
 * 
 * @return RSA_OP_SUCCESS on success, RSA_OP_FAILURE on failure.
 */
int decrypt_message(unsigned char* encrypted_message, size_t encrypted_len, 
                    const char* private_key_file,
                    unsigned char** decrypted_message,
                    size_t* decrypted_len)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    FILE *private_fp = NULL;
    unsigned char *decrypted = NULL;
    size_t outlen;
    int ret = RSA_OP_FAILURE;

    // Open the private key file
    private_fp = fopen(private_key_file, "rb");
    if (!private_fp) {
        fprintf(stderr, "Error opening private key file\n");
        goto cleanup;
    }

    // Read the private key
    pkey = PEM_read_PrivateKey(private_fp, NULL, NULL, NULL);
    if (!pkey) {
        fprintf(stderr, "Error reading private key\n");
        goto cleanup;
    }

    // Create context for decryption
    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) {
        fprintf(stderr, "Error creating context\n");
        goto cleanup;
    }

    // Initialize decryption
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        fprintf(stderr, "Error initializing decryption\n");
        goto cleanup;
    }

    // Set padding
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        fprintf(stderr, "Error setting padding\n");
        goto cleanup;
    }

    // Determine buffer length
    if (EVP_PKEY_decrypt(ctx, NULL, &outlen, encrypted_message, encrypted_len) <= 0) {
        fprintf(stderr, "Error determining buffer length\n");
        goto cleanup;
    }

    // Allocate buffer
    decrypted = (unsigned char*)malloc(outlen);
    if (!decrypted) {
        fprintf(stderr, "Error allocating memory\n");
        goto cleanup;
    }

    // Perform decryption
    if (EVP_PKEY_decrypt(ctx, decrypted, &outlen, encrypted_message, encrypted_len) <= 0) {
        fprintf(stderr, "Error decrypting message\n");
        free(decrypted);
        goto cleanup;
    }

    *decrypted_message = decrypted;
    *decrypted_len = outlen;
    ret = RSA_OP_SUCCESS;

cleanup:
    if (private_fp) fclose(private_fp);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pkey) EVP_PKEY_free(pkey);

    return ret;
}


// Charge un fichier de clé rsa vers la chaine dest
void load_rsa_key(char* rsa_key, char* dest, size_t t_max, uint32_t* t_read) {
    FILE* file = fopen(rsa_key, "rb"); // Open in binary mode

    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", rsa_key);
        exit(EXIT_FAILURE);
    }

    // Seek to the end of the file to get the file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file); // rewind back to the beginning

    // Check if file size is larger than authorized size
    if (file_size > t_max) {
        fprintf(stderr, "Error: File size exceeds maximum size (%zu)\n",
                                                                    t_max);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Read the file content into dest
    size_t bytes_read = fread(dest, 1, file_size, file);

    // Check if all bytes were read successfully
    if (bytes_read != file_size) {
        fprintf(stderr, "Error reading file: %s\n", rsa_key);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    *(t_read) = bytes_read;

    // Add null terminator to the end of the string
    dest[bytes_read] = '\0';

    fclose(file);
}

