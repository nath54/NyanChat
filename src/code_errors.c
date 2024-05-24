#include "code_errors.h"
#include "lib_chks.h"
#include "useful_lib.h"
#include "bits.h"

int c = 8;  // Nombre de bits de parité
int n = 16; // Taille du mot de code
int k = 8;  // Taille du mot d'information

// Matrice génératrice de X^8 + X^7 + X^5 + X^4 + X^3 + 1
uint16_t G[8][16] = {
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
uint16_t H[8][16] = {
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
uint16_t S[256] = { 0b11111111 };

/*
Renvoie un mot du code sur 8 + c bits (c étant le degré de votre polynôme),
rajoutant au mot m les bits de parité décrit par la matrice G dérivée de votre code polynomial.
Le mot à encoder (sur 8 bits) sera placé sur les premiers bits de la variable m et complété
par 8 bits de padding avant d’être fourni en argument à la fonction.
Vous privilégierez des opérateurs bit à bit en évitant les opérations arithmétiques.
*/
uint16_t encode(uint16_t G[8][16], uint16_t m)
{
    for (int j = 8; j < 16; j++) {
        uint16_t parity_bit = 0;
        for (int i = 0; i < 8; i++)
            parity_bit ^= G[i][j] & get_nth_bit(i, m);

        if (parity_bit)
            m = set_nth_bit(j, m);
    }
    return m;
}

int code_hamming_distance(uint16_t G[8][16])
{
    int distance = 8;
    int N = 1 << 8;
    for (int i = 1; i < N; i++) {
        uint16_t word = encode(G, i << 8);
        int w = weight(word);
        if (w < distance)
            distance = w;
    }
    return distance;
}

void create_check_matrix(uint16_t G[8][16], uint16_t H[8][16])
{
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // A transpose matrix
            H[i][j] = G[j][i+8];
            // Identity matrix
            H[i][j+8] = (i == j) ? 1 : 0;
        }
    }
}

uint16_t shift_register(uint16_t p, uint16_t x)
{
    uint16_t remainder = 0;
    (void)p;
    (void)x;
    return remainder;
}

void create_generator_matrix(uint16_t G[8][16], uint16_t p)
{
    uint16_t remainder;
    for (int i = 0; i < k; i++) {
        remainder = shift_register(p, 1 << (n-1-i));
        for (int j = 0; j < k; j++) {
            G[i][j] = (i == j) ? 1 : 0; // Identity matrix
        }
        for (int j = 0; j < c; j++) {
            G[i][j+k] = get_nth_bit(j, remainder);
        }
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
    for (uint32_t c = 0; c < msg->msg_length; c++) {
        // Cast the word to (possibly) add errors to it
        word = (uint16_t)(msg->msg[c]) << 8;
        for (int b = 0; b < 8; b++) {
            if (randint(1000) < BIT_ERROR_RATE)
                // Add an error to the message
                word = chg_nth_bit(b, word);
        }
        msg->msg[c] = (char)(word >> 8);
    }
}

