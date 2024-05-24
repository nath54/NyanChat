#include "code_errors.h"
#include "lib_chks.h"
#include "bits.h"

uint16_t P = 0b110111001;  // X^8 + X^7 + X^5 + X^4 + X^3 + 1
const int HD = 4; // Hamming distance of P
const int CORRECTION = 1;
const int DETECTION = 3;

// Matrice génératrice de P
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

void create_generator_matrix(uint16_t G[K][N], uint16_t P)
{
    uint16_t remainder;
    for (int i = 0; i < K; i++) {
        remainder = rem_lfsr(P, 1 << (N-1-i));
        for (int j = 0; j < K; j++) {
            G[i][j] = (i == j) ? 1 : 0; // Identity matrix
        }
        for (int j = 0; j < C; j++) {
            G[i][j+K] = get_nth_bit(j, remainder);
        }
    }
}

void create_check_matrix(uint16_t G[K][N], uint16_t H[C][N])
{
    for (int i = 0; i < C; i++) {
        for (int j = 0; j < K; j++)
            H[i][j] = G[j][i+K];  // A transpose matrix
        for (int j = 0; j < C; j++)
            H[i][j+K] = (i == j) ? 1 : 0;  // Identity matrix
    }
}

void create_syndrome_array(uint16_t P, uint16_t S[Nc])
{
    for (int e = 0; e < Nk; e++) {
        uint8_t syndrome = rem_lfsr(P, e);
        S[syndrome] &= e;
    }
}

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

uint16_t rem_lfsr(uint16_t P, uint16_t x)
{
    for (int i = 0; i < K; i++) {
        x = x >> 1;
        x ^= P;
    }
    return x;
}

uint16_t encode_lfsr(uint16_t P, char m)
{
    return ((uint16_t)m << C) + rem_lfsr(P, m);
}

int code_hamming_distance(uint16_t P)
{
    int distance = K;
    for (int i = 1; i < Nk; i++) {
        uint16_t word = encode_lfsr(P, i);
        int w = weight(word);
        if (w < distance)
            distance = w;
    }
    return distance;
}

/* For Client and Proxy */

void add_control_bits(Message *msg)
{
    for (uint32_t i = 0; i < msg->msg_length; i++) {
        uint16_t encoded = encode_lfsr(P, msg->msg[i]);
        msg->control[i] = encoded & (Nc - 1);
    }
}

int code_correct_error(Message* msg)
{
    int rc = 0;

    for (uint32_t i = 0; i < msg->msg_length; i++) {
        uint16_t syndrome = rem_lfsr(P, msg->msg[i]);
        uint16_t err = S[syndrome];

        if (weight(syndrome) <= CORRECTION)
            msg->error[i] = false;
        else {
            msg->error[i] = true;
            rc = -1;
        }
        msg->msg[i] ^= err;  // correct errors
    }
    return rc;
}

void code_insert_error(Message* msg)
{
    uint16_t byte;
    for (uint32_t i = 0; i < msg->msg_length; i++) {
        // Cast the byte to (possibly) add errors to it
        byte = (uint16_t)msg->msg[i] << C;
        for (int b = 0; b < N; b++) {
            if ((rand() / RAND_MAX) < BIT_ERROR_RATE)
                byte = chg_nth_bit(b, byte);  // Add an error to the message
        }
        msg->msg[i] = byte >> C;
    }
}
