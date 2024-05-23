#include "code_errors.h"
#include "lib_chks.h"
#include "useful_lib.h"
#include "bits.h"

int K = 8;  // Nombre d'informations

/*
Renvoie un mot du code sur 8 + c bits (c étant le degré de votre polynôme),
rajoutant au mot m les bits de parité décrit par la matrice G dérivée de votre code polynomial.
Le mot à encoder (sur 8 bits) sera placé sur les premiers bits de la variable m et complété
par 8 bits de padding avant d’être fourni en argument à la fonction.
Vous privilégierez des opérateurs bit à bit en évitant les opérations arithmétiques.
*/
uint16_t encode(uint16_t** g, int n, uint16_t m)
{
    uint16_t m_encode = m << (n-K);
    for (int i = K; i < n; i++) {
        uint16_t parity_bit = 0;
        for (int j = 0; j < K; j++)
            parity_bit ^= g[j][i] & get_nth_bit(n-j-1, m);

        set_nth_bit(n-i-1, m_encode);
    }
    return m_encode;
}

int code_hamming_distance(uint16_t **g, int n)
{
    int distance = K;
    int N = 1 << K;  // numbers of words
    for (int i = 0; i < N; i++) {
        uint16_t word = encode(g, n, i);
        int w = weight(word);
        if (w < distance)
            distance = w;
    }
    return distance;
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


void code_add_errors_to_msg(Message* msg, int nb_errors)
{
    int word_to_change;
    uint16_t erroned_word;
    for (int i = 0; i < nb_errors; i++) {
        // Choose a random word to change
        word_to_change = randint(msg->msg_length);
        // Add an error to the message
        erroned_word = chg_nth_bit(randint(K), (uint16_t)(msg->msg[word_to_change]));
        msg->msg[word_to_change] = erroned_word;
    }
}

