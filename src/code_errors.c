#include "code_errors.h"
#include "lib_chks.h"
#include "useful_lib.h"
#include "bits.h"

// Matrice génératrice de X^8 + X^7 + X^5 + X^4 + X^3 + 1
uint16_t G[K][N] = {
    {1,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0},
    {0,1,0,0,0,0,0,0,0,0,1,1,1,0,1,1},
    {0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1},
    {0,0,0,1,0,0,0,0,1,0,1,1,1,1,0,0},
    {0,0,0,0,1,0,0,0,0,1,0,1,1,1,1,0},
    {0,0,0,0,0,1,0,0,0,0,1,0,1,1,1,1},
    {0,0,0,0,0,0,1,0,1,1,0,0,1,0,1,1},
    {0,0,0,0,0,0,0,1,1,0,1,1,1,0,0,1}
};
// Matrice de test
uint16_t H[K][N] = {
    {0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0},
    {1,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0},
    {1,1,0,1,0,1,0,1,0,0,1,0,0,0,0,0},
    {1,1,0,1,1,0,0,1,0,0,0,1,0,0,0,0},
    {0,1,0,1,1,1,1,1,0,0,0,0,1,0,0,0},
    {1,0,0,1,1,1,0,0,0,0,0,0,0,1,0,0},
    {1,1,0,0,1,1,1,0,0,0,0,0,0,0,1,0},
    {0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,1}
};

// Matrice associant les syndromes aux erreurs
uint16_t S[Nc] = { 0b11111111 };


uint16_t encode(uint16_t G[K][N], uint16_t m)
{
    for (int j = K; j < N; j++) {
        uint16_t parity_bit = 0;
        for (int i = 0; i < C; i++)
            parity_bit ^= G[i][j] & get_nth_bit(i, m);

        if (parity_bit)
            m = set_nth_bit(j, m);
    }
    return m;
}


int code_hamming_distance(uint16_t G[K][N])
{
    int distance = K;
    for (int i = 1; i < Nk; i++) {
        uint16_t word = encode(G, i << K);
        int w = weight(word);
        if (w < distance)
            distance = w;
    }
    return distance;
}

void create_check_matrix(uint16_t G[K][N], uint16_t H[K][N])
{
    for (int i = 0; i < C; i++) {
        for (int j = 0; j < K; j++) {
            H[i][j] = G[j][i+K];  // A transpose matrix
            if (j < C)
                H[i][j+K] = (i == j) ? 1 : 0;  // Identity matrix
        }
    }
}

uint16_t shift_register(uint16_t p, uint16_t x)
{
    for (int i = 0; i < C; i++) {
        x = x >> 1;
        x ^= p;
    }
    return x;
}


void create_generator_matrix(uint16_t G[K][N], uint16_t p)
{
    uint16_t remainder;
    for (int i = 0; i < K; i++) {
        remainder = shift_register(p, 1 << (N-1-i));
        for (int j = 0; j < K; j++) {
            G[i][j] = (i == j) ? 1 : 0; // Identity matrix
        }
        for (int j = 0; j < C; j++) {
            G[i][j+K] = get_nth_bit(j, remainder);
        }
    }
}

void create_syndrome_array(uint16_t p, uint16_t S[Nc])
{
    for (int e = 0; e < Nn; e++) {
        uint8_t syndrome = shift_register(p, e);
        S[syndrome] &= e;
    }
}

// Function to detect an error in the message
// Returns 0 if no errors are detected,
//         1 if errors are detected and can be corrected
//         2 if errors are detected but cannot be corrected
int code_detect_error(Message* msg, uint16_t *err)
{
    // TODO
    (void)msg;
    (void)err;
    return 0;
}


// Function that directly corrects the error in msg
// Returns 0 if everything went well
// Otherwise, returns -1
int code_correct_error(Message* msg, uint16_t err)
{
    (void)msg;
    (void)err;
    // TODO: compléter cette fonction

    return 0;
}


void code_insert_error(Message* msg)
{
    uint16_t word;
    for (uint32_t i = 0; i < msg->msg_length; i++) {
        // Cast the word to (possibly) add errors to it
        word = (uint16_t)(msg->msg[i]) << 8;
        for (int b = 0; b < 8; b++) {
            if (randint(1000) < BIT_ERROR_RATE)
                // Add an error to the message
                word = chg_nth_bit(b, word);
        }
        msg->msg[i] = (char)(word >> 8);
    }
}

