#pragma once

// To include the Message struct
#include "tcp_connection.h"


#define BIT_ERROR_RATE 0.1    // In %, the rate of creating errors


// Encoding function
uint16_t encode(uint16_t** g, int n, uint16_t m);

/**
 * @brief Calculate the Hamming distance of the polynomial code
 * 
 * @param g the generator matrix representing the polynomial code
 * @return `int` the minimal distance between words 0 and any other word
 */
int code_hamming_distance(uint16_t **g, int l);

// Function to detect an error in the message
// Returns 0 if no errors are detected,
// otherwise a positive value that can for example indicate whether or not it is possible to
// correct the error, idk if it's possible
int code_detect_error(Message* msg);


// Function that directly corrects the error in msg
// Returns 0 if everything went well
// Otherwise, returns -1
int code_correct_error(Message* msg);


/**
 * @brief Add noises to a message.
 * @note This function is called by the proxy.
 * 
 * @param msg The message to add noise to.
 */
void code_add_errors_to_msg(Message* msg);

