#include <stdio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

int generate_keypair(const char* private_key_file, const char* public_key_file,
                     int key_size)
{

    BIGNUM *bne = NULL;
    RSA *rsa = NULL;
    BIO *pri_bio = NULL, *pub_bio = NULL;

    bne = BN_new();
    if (!BN_set_word(bne, RSA_F4)) {
        fprintf(stderr, "Failed to set BN value\n");
        goto error;
    }

    rsa = RSA_generate_key(key_size, BN_as_u32(bne), NULL, NULL);
    if (!rsa) {
        fprintf(stderr, "Failed to generate RSA key\n");
        goto error;
    }

    pri_bio = BIO_new_file(private_key_file, "w");
    pub_bio = BIO_new_file(public_key_file, "w");
    if (!pri_bio || !pub_bio) {
        fprintf(stderr, "Failed to create key files\n");
        goto error;
    }

    PEM_write_PrivateKey(pri_bio, rsa, NULL, NULL, 0, NULL, NULL);
    PEM_write_PublicKey(pub_bio, rsa);

    RSA_free(rsa);
    BN_free(bne);
    BIO_free_all(pri_bio);
    BIO_free_all(pub_bio);

    return 1;

error:
    if (rsa) RSA_free(rsa);
    if (bne) BN_free(bne);
    if (pri_bio) BIO_free_all(pri_bio);
    if (pub_bio) BIO_free_all(pub_bio);
    return 0;
}

/*

Use example:

    generate_keypair("private.pem", "public.pem", 2048);

*/


int encrypt_message(const char* message, size_t message_len, 
                    FILE* public_key_file, unsigned char** encrypted_message,
                    size_t* encrypted_len)
{

    RSA *public_key = NULL;
    BIO *pub_bio = NULL;
    int padding = RSA_PKCS1_PADDING; // Use PKCS#1 v1.5 padding

    // Read public key
    pub_bio = BIO_new_file(public_key_file, "rb");
    if (!pub_bio) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    public_key = PEM_read_RSAPublicKey(pub_bio, NULL, NULL, NULL);
    BIO_free_all(pub_bio);
    if (!public_key) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // Determine required buffer size for encrypted message
    *encrypted_len = RSA_size(public_key);

    // Allocate memory for encrypted message
    *encrypted_message = malloc(*encrypted_len);
    if (!*encrypted_message) {
        RSA_free(public_key);
        return -1;
    }

    // Perform encryption
    int result = RSA_public_encrypt(message_len, (unsigned char*)message, 
                                    *encrypted_message, public_key, padding);
    RSA_free(public_key);
    if (result == -1) {
        ERR_print_errors_fp(stderr);
        free(*encrypted_message);
        return -1;
    }

    return 0;
}



int decrypt_message(unsigned char* encrypted_message, size_t encrypted_len, 
                     FILE* private_key_file, unsigned char** decrypted_message,
                     size_t* decrypted_len)
{

    RSA *private_key = NULL;
    BIO *pri_bio = NULL;
    int padding = RSA_PKCS1_PADDING; // Use PKCS#1 v1.5 padding similar to encryption

    // Read private key
    pri_bio = BIO_new_file(private_key_file, "rb");
    if (!pri_bio) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    private_key = PEM_read_RSAPrivateKey(pri_bio, NULL, NULL, NULL);
    BIO_free_all(pri_bio);
    if (!private_key) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // Determine required buffer size for decrypted message
    *decrypted_len = RSA_size(private_key);

    // Allocate memory for decrypted message
    *decrypted_message = malloc(*decrypted_len);
    if (!*decrypted_message) {
        RSA_free(private_key);
        return -1;
    }

    // Perform decryption
    int result = RSA_private_decrypt(encrypted_len, encrypted_message, 
                                    *decrypted_message, private_key, padding);
    RSA_free(private_key);
    if (result == -1) {
        ERR_print_errors_fp(stderr);
        free(*decrypted_message);
        return -1;
    }

    *decrypted_len = result; // Update decrypted_len with actual decrypted message size

    return 0;
}


