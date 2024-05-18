#include <stdio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>

// Gemini code corrected by ChatGpt-4o


/**
 * Generates an RSA key pair and saves the private and public keys
 *  to the specified files.
 * 
 * @param private_key_file The file to save the private key.
 * @param public_key_file The file to save the public key.
 * @param key_size The size of the RSA key in bits.
 * 
 * @return 1 on success, 0 on failure.
 * 
 * This function generates an RSA key pair using the OpenSSL EVP API.
 * It creates an EVP_PKEY context, sets the RSA key size,
 * and generates the key pair. The keys are then written to
 * the specified files in PEM format.
 * All resources are cleaned up before returning.
 * 
 * 
 * Example use:

    > generate_keypair("private.pem", "public.pem", 2048);

 */
int generate_keypair(const char* private_key_file, const char* public_key_file,
                     int key_size)
{
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    BIO *pri_bio = NULL, *pub_bio = NULL;
    int ret = 0;

    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        fprintf(stderr, "EVP_PKEY_CTX_new_id failed\n");
        goto error;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        fprintf(stderr, "EVP_PKEY_keygen_init failed\n");
        goto error;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_size) <= 0) {
        fprintf(stderr, "EVP_PKEY_CTX_set_rsa_keygen_bits failed\n");
        goto error;
    }

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "EVP_PKEY_keygen failed\n");
        goto error;
    }

    pri_bio = BIO_new_file(private_key_file, "w");
    pub_bio = BIO_new_file(public_key_file, "w");
    if (!pri_bio || !pub_bio) {
        fprintf(stderr, "BIO_new_file failed\n");
        goto error;
    }

    if (PEM_write_bio_PrivateKey(pri_bio, pkey,
                                 NULL, NULL, 0, NULL, NULL) <= 0)
    {
        fprintf(stderr, "PEM_write_bio_PrivateKey failed\n");
        goto error;
    }
    if (PEM_write_bio_PUBKEY(pub_bio, pkey) <= 0) {
        fprintf(stderr, "PEM_write_bio_PUBKEY failed\n");
        goto error;
    }

    ret = 1; // Success

error:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pkey) EVP_PKEY_free(pkey);
    if (pri_bio) BIO_free_all(pri_bio);
    if (pub_bio) BIO_free_all(pub_bio);

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
 * @return 0 on success, -1 on failure.
 * 
 * This function reads the public key from a PEM file, initializes an EVP_PKEY
 * context for encryption, and performs the encryption
 * using RSA with PKCS#1 v1.5 padding. The encrypted message and its length
 * are returned through the output parameters.
 */
int encrypt_message(const char* message, size_t message_len,
                    const char* public_key_file,
                    unsigned char** encrypted_message, size_t* encrypted_len)
{
    EVP_PKEY *public_key = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    BIO *pub_bio = NULL;
    int ret = -1;

    pub_bio = BIO_new_file(public_key_file, "r");
    if (!pub_bio) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    public_key = PEM_read_bio_PUBKEY(pub_bio, NULL, NULL, NULL);
    BIO_free_all(pub_bio);
    if (!public_key) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    ctx = EVP_PKEY_CTX_new(public_key, NULL);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (EVP_PKEY_encrypt(ctx, NULL, encrypted_len,
                         (unsigned char*)message, message_len) <= 0)
    {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    *encrypted_message = (unsigned char*)malloc(*encrypted_len);
    if (!*encrypted_message) {
        fprintf(stderr, "Memory allocation failed\n");
        goto cleanup;
    }

    if (EVP_PKEY_encrypt(ctx, *encrypted_message, encrypted_len,
                         (unsigned char*)message, message_len) <= 0)
    {
        ERR_print_errors_fp(stderr);
        free(*encrypted_message);
        *encrypted_message = NULL;
        *encrypted_len = 0;
        goto cleanup;
    }

    ret = 0; // Success

cleanup:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (public_key) EVP_PKEY_free(public_key);

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
 * @return 0 on success, -1 on failure.
 * 
 * This function reads the private key from a PEM file,
 * initializes an EVP_PKEY context for decryption,
 * and performs the decryption using RSA with PKCS#1 v1.5 padding.
 * The decrypted message and its length
 * are returned through the output parameters.
 */
int decrypt_message(const unsigned char* encrypted_message,
                    size_t encrypted_len,
                    const char* private_key_file,
                    unsigned char** decrypted_message,
                    size_t* decrypted_len)
{
    EVP_PKEY *private_key = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    BIO *pri_bio = NULL;
    int ret = -1;

    pri_bio = BIO_new_file(private_key_file, "r");
    if (!pri_bio) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    private_key = PEM_read_bio_PrivateKey(pri_bio, NULL, NULL, NULL);
    BIO_free_all(pri_bio);
    if (!private_key) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    ctx = EVP_PKEY_CTX_new(private_key, NULL);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (EVP_PKEY_decrypt(ctx, NULL, decrypted_len,
                         encrypted_message, encrypted_len) <= 0)
    {
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    *decrypted_message = (unsigned char*)malloc(*decrypted_len);
    if (!*decrypted_message) {
        fprintf(stderr, "Memory allocation failed\n");
        goto cleanup;
    }

    if (EVP_PKEY_decrypt(ctx, *decrypted_message, decrypted_len,
                         encrypted_message, encrypted_len) <= 0)
    {
        ERR_print_errors_fp(stderr);
        free(*decrypted_message);
        *decrypted_message = NULL;
        *decrypted_len = 0;
        goto cleanup;
    }

    ret = 0; // Success

cleanup:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (private_key) EVP_PKEY_free(private_key);

    return ret;
}




