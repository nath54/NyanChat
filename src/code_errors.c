#include "code_errors.h"
#include "lib_chks.h"
#include "bits.h"

uint16_t P = 0b110111001;  // X^8 + X^7 + X^5 + X^4 + X^3 + 1
const int HD = 4; // Hamming distance of P
const int CORRECTION = 1;
const int DETECTION = 3;

double BIT_ERROR_RATE = 0.01; // in % 

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
uint16_t S[Nc];

void create_generator_matrix(uint16_t G[K][N], uint16_t P)
{
    for (int i = 0; i < K; i++) {
        uint8_t remainder = decode_lfsr(P, 1 << (N-1-i));
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

void create_syndrome_array()
{
    for (uint16_t i = 0; i < Nc; i++)
        S[i] = Nc-1;

    for (uint16_t e = 0; e < Nc; e++) {
        uint8_t syndrome = decode_lfsr(P, e << C);
        printf("Syndrome %d : %d\n", syndrome, e);
        S[syndrome] &= e;
    }
}

uint16_t encode(uint16_t G[K][N], uint16_t m)
{
    for (int j = K; j < N; j++) {
        uint8_t parity_bit = 0;
        for (int i = 0; i < K; i++)
            parity_bit ^= G[i][j] & get_nth_bit(i, m);

        if (parity_bit)
            m = set_nth_bit(j, m);
    }
    return m;
}


uint8_t decode(uint16_t H[C][N], uint16_t m)
{
    uint8_t syndrome = 0;
    for (int i = 0; i < C; i++) {
        uint8_t parity_bit = 0;
        for (int j = 0; j < N; j++)
            parity_bit ^= H[i][j] & get_nth_bit(j, m);
        if (parity_bit)
            syndrome = set_nth_bit(i+C, syndrome);
    }
    return syndrome;
}

uint16_t encode_lfsr(uint16_t P, uint16_t m)
{
    return m + decode_lfsr(P, m);
}

uint8_t decode_lfsr(uint16_t P, uint16_t m)
{
    uint16_t r = m;
    P = P << (K-1);
    for (int i = 0; i < C; i++) {
        if (get_nth_bit(i, r))
            r ^= P;
        P = P >> 1;
    }
    return r;
}

int code_hamming_distance(uint16_t P)
{
    int distance = K;
    for (int i = 1; i < Nk; i++) {
        uint16_t word = encode_lfsr(P, i << C);
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
        uint8_t control = decode_lfsr(P, msg->msg[i] << C);
        msg->control[i] = control;
    }
}

int code_correct_error(Message* msg)
{
    int rc = 0;

    for (uint32_t i = 0; i < msg->msg_length; i++) {
        uint16_t word = ((uint16_t)msg->msg[i] << C) + (uint8_t)msg->control[i];
        uint8_t syndrome = decode_lfsr(P, word);
        uint16_t err = S[syndrome];
        if (weight(err) <= CORRECTION)
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
    uint16_t codeword;
    for (uint32_t i = 0; i < msg->msg_length; i++) {
        // Cast the word to (possibly) add errors to it
        codeword = ((uint16_t)msg->msg[i] << C); // + msg->control[i];
        for (int b = 0; b < N; b++) {
            if (((double)rand() / RAND_MAX) < BIT_ERROR_RATE)
                codeword = chg_nth_bit(b, codeword);  // Add an error
        }
        msg->msg[i] = codeword >> C;
        // msg->control[i] = codeword & (Nc - 1);
    }
}
