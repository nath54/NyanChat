#include "code_errors.h"
#include "lib_chks.h"
#include "useful_lib.h"
#include "bits.h"


/*
Renvoie un mot du code sur 8 + c bits (c étant le degré de votre polynôme),
rajoutant au mot m les bits de parité décrit par la matrice G dérivée de votre code polynomial.
Le mot à encoder (sur 8 bits) sera placé sur les premiers bits de la variable m et complété
par 8 bits de padding avant d’être fourni en argument à la fonction.
Vous privilégierez des opérateurs bit à bit en évitant les opérations arithmétiques.
*/
uint16_t encode_G(uint16_t **g, int l, uint16_t m)
{
    // TODO: compléter cette fonction
    uint16_t coef, res = 0;
    for (int i = 0; i < l; i++) {
        coef = 0;
        for (int j = 0; j < 8; j++)
            { coef += g[j][i] * get_nth_bit(l-j-1, m); }
        coef %= 2;
        res += (coef << (l-i-1));
    }
    return res;
}

int code_hamming_distance(uint16_t **g, int l)
{
    uint16_t test;
    int min_weight = 8, nb_bits = 0;
    for (int i = 0; i < 256; i++) {
        test = encode_G(g, l, i);
        nb_bits = card_word_bits(test);
        if (nb_bits < min_weight)
            { min_weight = nb_bits; }
    }
    return min_weight;
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
        erroned_word = chg_nth_bit(randint(8), (uint16_t)(msg->msg[word_to_change]));
        msg->msg[word_to_change] = erroned_word;
    }
}

