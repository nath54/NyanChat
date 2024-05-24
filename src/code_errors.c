#include "code_errors.h"
#include "lib_chks.h"
#include "useful_lib.h"
#include "bits.h"

int c = 8;  // Nombre de bits de parité

/*
Renvoie un mot du code sur 8 + c bits (c étant le degré de votre polynôme),
rajoutant au mot m les bits de parité décrit par la matrice G dérivée de votre code polynomial.
Le mot à encoder (sur 8 bits) sera placé sur les premiers bits de la variable m et complété
par 8 bits de padding avant d’être fourni en argument à la fonction.
Vous privilégierez des opérateurs bit à bit en évitant les opérations arithmétiques.
*/
uint16_t encode_G(uint16_t** g, uint16_t m)
{
    for (int j = 8; j < 16; j++) {
        uint16_t parity_bit = 0;
        for (int i = 0; i < 8; i++)
            parity_bit ^= g[i][j] & get_nth_bit(i, m);

        if (parity_bit)
            m = set_nth_bit(j, m);
    }
    return m;
}

int code_hamming_distance(uint16_t **g)
{
    int distance = 8;
    for (int i = 1; i < 256; i++) {
        uint16_t word = encode_G(g, i << 8);
        int w = weight(word);
        if (w < distance)
            distance = w;
    }
    return distance;
}

void create_check_matrix(uint16_t **g, uint16_t **h)
{
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // A transpose matrix
            h[i][j] = g[j][i+8];
            // Identity matrix
            h[i][j+8] = (i == j) ? 1 : 0;
        }
    }
}

uint16_t shift_register(uint16_t p, uint16_t m)
{
    uint16_t remainder = 0;
    (void)p;
    (void)m;
    return remainder;
}

void create_generator_matrix(uint16_t **g, uint16_t p)
{
    uint16_t remainder;
    for (int i = 8; i < 16; i++) {
        remainder = shift_register(p, i << 8);
        g[i-8][i-8] = 1; // Identity matrix
        for (int j = 8; j < 16; j++)
            { g[i-8][j] = get_nth_bit(j-8, remainder); }
    }
}

// Function to detect an error in the message
// Returns 0 if no errors are detected,
// otherwise a positive value that can for example indicate whether or not it is possible to
// correct the error, idk if it's possible
int code_detect_error(Message* msg)
{
    (void)msg;
    
    // TODO: compléter cette fonction

    return 0;
}


// Function that directly corrects the error in msg
// Returns 0 if everything went well
// Otherwise, returns -1
int code_correct_error(Message* msg)
{
    (void)msg;

    // TODO: compléter cette fonction

    return 0;
}


void code_add_errors_to_msg(Message* msg)
{
    uint16_t word;
    for (uint32_t c = 0; c < msg->msg_length; c++) {
        // Cast the word to (possibly) add errors to it
        word = (uint16_t)(msg->msg[c]) << 8;
        for (int b = 0; b < 8; b++) {
            if (randint(100) < BIT_ERROR_RATE * 100)
                // Add an error to the message
                word = chg_nth_bit(b, word);
        }
        msg->msg[c] = (char)(word >> 8);
    }
}

