#include <stdio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>

// Gemini code corrected by ChatGpt-4o
int generate_keypair(const char* private_key_file, const char* public_key_file,
                     int key_size)
{
    BIGNUM *bne = NULL;
    RSA *rsa = NULL;
    BIO *pri_bio = NULL, *pub_bio = NULL;
    int ret = 0;

    bne = BN_new();
    if (!bne) {
        fprintf(stderr, "BN_new failed\n");
        goto error;
    }
    if (BN_set_word(bne, RSA_F4) != 1) {
        fprintf(stderr, "BN_set_word failed\n");
        goto error;
    }

    rsa = RSA_new();
    if (!rsa) {
        fprintf(stderr, "RSA_new failed\n");
        goto error;
    }

    if (RSA_generate_key_ex(rsa, key_size, bne, NULL) != 1) {
        fprintf(stderr, "RSA_generate_key_ex failed\n");
        goto error;
    }

    pri_bio = BIO_new_file(private_key_file, "w");
    pub_bio = BIO_new_file(public_key_file, "w");
    if (!pri_bio || !pub_bio) {
        fprintf(stderr, "BIO_new_file failed\n");
        goto error;
    }

    if (PEM_write_bio_RSAPrivateKey(pri_bio, rsa,
                                    NULL, NULL, 0, NULL, NULL) != 1)
    {
        fprintf(stderr, "PEM_write_bio_RSAPrivateKey failed\n");
        goto error;
    }
    if (PEM_write_bio_RSAPublicKey(pub_bio, rsa) != 1) {
        fprintf(stderr, "PEM_write_bio_RSAPublicKey failed\n");
        goto error;
    }

    ret = 1; // Success

error:
    if (bne) BN_free(bne);
    if (rsa) RSA_free(rsa);
    if (pri_bio) BIO_free_all(pri_bio);
    if (pub_bio) BIO_free_all(pub_bio);

    return ret;
}

/*

Use example:

    generate_keypair("private.pem", "public.pem", 2048);

*/

int encrypt_message(const char* message, size_t message_len,
                    const char* public_key_file,
                    unsigned char** encrypted_message, size_t* encrypted_len)
{
    RSA *public_key = NULL;
    BIO *pub_bio = NULL;
    int padding = RSA_PKCS1_PADDING;
    int result = -1;

    // Read public key
    pub_bio = BIO_new_file(public_key_file, "r");
    if (!pub_bio) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    public_key = PEM_read_bio_RSAPublicKey(pub_bio, NULL, NULL, NULL);
    BIO_free_all(pub_bio);
    if (!public_key) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // Determine required buffer size for encrypted message
    *encrypted_len = RSA_size(public_key);

    // Allocate memory for encrypted message
    *encrypted_message = (unsigned char*)malloc(*encrypted_len);
    if (!*encrypted_message) {
        RSA_free(public_key);
        return -1;
    }

    // Perform encryption
    result = RSA_public_encrypt(message_len, (unsigned char*)message,
                                *encrypted_message, public_key, padding);
    if (result == -1) {
        ERR_print_errors_fp(stderr);
        free(*encrypted_message);
        *encrypted_message = NULL;
        *encrypted_len = 0;
    } else {
        *encrypted_len = result;
    }

    RSA_free(public_key);
    return (result == -1) ? -1 : 0;
}



int decrypt_message(const unsigned char* encrypted_message,
                    size_t encrypted_len,
                    const char* private_key_file,
                    unsigned char** decrypted_message,
                    size_t* decrypted_len)
{
    RSA *private_key = NULL;
    BIO *pri_bio = NULL;
    int padding = RSA_PKCS1_PADDING;
    int result = -1;

    // Read private key
    pri_bio = BIO_new_file(private_key_file, "r");
    if (!pri_bio) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    private_key = PEM_read_bio_RSAPrivateKey(pri_bio, NULL, NULL, NULL);
    BIO_free_all(pri_bio);
    if (!private_key) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // Determine required buffer size for decrypted message
    size_t rsa_size = RSA_size(private_key);

    // Allocate memory for decrypted message
    *decrypted_message = (unsigned char*)malloc(rsa_size);
    if (!*decrypted_message) {
        RSA_free(private_key);
        return -1;
    }

    // Perform decryption
    result = RSA_private_decrypt(encrypted_len, encrypted_message,
                                 *decrypted_message, private_key, padding);
    if (result == -1) {
        ERR_print_errors_fp(stderr);
        free(*decrypted_message);
        *decrypted_message = NULL;
        *decrypted_len = 0;
    } else {
        *decrypted_len = result;
    }

    RSA_free(private_key);
    return (result == -1) ? -1 : 0;
}



