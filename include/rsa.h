#pragma once

#include <stdio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>


int generate_keypair(const char* private_key_file, const char* public_key_file,
                     int key_size);
/*

Use example:

    generate_keypair("private.pem", "public.pem", 2048);

*/


int encrypt_message(const char* message, size_t message_len, 
                    FILE* public_key_file, unsigned char** encrypted_message,
                    size_t* encrypted_len);


int decrypt_message(unsigned char* encrypted_message, size_t encrypted_len, 
                     FILE* private_key_file, unsigned char** decrypted_message,
                     size_t* decrypted_len);

