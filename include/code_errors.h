#pragma once

// To include the Message struct
#include "tcp_connection.h"


#define BIT_ERROR_RATE 10    // In per thousand, the rate of creating errors


// Encoding function
uint16_t encode(uint16_t G[8][16], uint16_t m);

/**
 * @brief Calculate the Hamming distance of the polynomial code.
 * 
 * @param g the generator matrix representing the polynomial code
 * @return `int` the minimal distance between word 0 and any other word.
 */
int code_hamming_distance(uint16_t G[8][16]);

/*
Calculate the remainder of the division of the word `x`
by the polynomial `p` using the shift register method.
*/
uint16_t shift_register(uint16_t p, uint16_t x);

// Fill the generator matrix `G` from the polynomial `p`.
void create_generator_matrix(uint16_t G[8][16], uint16_t p);

void create_syndrome_array(uint16_t p, uint16_t S[256]);

// Function to detect an error in the message
// Returns 0 if no errors are detected,
// otherwise a positive value that can for example indicate whether or not it is possible to
// correct the error, idk if it's possible
int code_detect_error(Message* msg, uint16_t* err);


// Function that directly corrects the error in msg
// Returns 0 if everything went well
// Otherwise, returns -1
int code_correct_error(Message* msg, uint16_t err);


/**
 * @brief Add errors to a message.
 * @note This function is called by the proxy.
 * 
 * @param msg The message to add noise into.
 */
void code_insert_error(Message* msg);

