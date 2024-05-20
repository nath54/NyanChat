#pragma once

#include <stdio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>


#define RSA_OP_SUCCESS 0
#define RSA_OP_FAILURE -1



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
                     int key_size);




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
                    size_t* encrypted_len);



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
                    size_t* decrypted_len);




// Charge un fichier de clé rsa vers la chaine dest
void load_rsa_key(char* rsa_key, char* dest, size_t t_max, uint32_t* t_read);