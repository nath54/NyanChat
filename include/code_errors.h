#pragma once

#include "tcp_connection.h"

#define BIT_ERROR_RATE 0.01  // In %
#define N 16 // Taille du mot de code
#define K 8  // Taille du mot d'information
#define C 8  // Nombre de bits de parité

#define Nc 256
#define Nk 256

/**
 * Renvoie un mot du code sur 8 + c bits (c étant le degré de votre polynôme),
 * rajoutant au mot m les bits de parité décrit par la matrice G dérivée de votre code polynomial.*
 * Le mot à encoder (sur 8 bits) sera placé sur les premiers bits de la variable m et complété
 * par 8 bits de padding avant d’être fourni en argument à la fonction.
 * Vous privilégierez des opérateurs bit à bit en évitant les opérations arithmétiques.
*/
uint16_t encode(uint16_t G[K][N], uint16_t m);

void add_control_bits(Message *msg);

/**
 * @brief Calculate the Hamming distance of the polynomial code.
 * 
 * @param P The polynomial generator
 * @return `int` the minimal distance between word 0 and any other word.
 */
int code_hamming_distance(uint16_t P);

/*
Calculate the remainder of the division of the word `x`
by the polynomial `p` using the shift register method.
*/
uint16_t rem_lfsr(uint16_t p, uint16_t x);

/**
 * Encode a 8-bit word with polynomial generator P using LFSR
*/
uint16_t encode_lfsr(uint16_t P, char m);

// Fill the generator matrix `G` from the polynomial `p`.
void create_generator_matrix(uint16_t G[K][N], uint16_t p);

void create_syndrome_array(uint16_t p, uint16_t S[Nc]);

// Function to detect an error in the message
// Returns 0 : No errors, or errors corrected
//         -1 : Errors detected but not corrected
int code_correct_error(Message* msg);

/**
 * @brief Add errors to a message.
 * @note This function is called by the proxy.
 * 
 * @param msg The message to add noise into.
 */
void code_insert_error(Message* msg);
