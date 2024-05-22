#include "code_errors.h"
#include "lib_chks.h"
#include "useful_lib.h"
#include "bits.h"

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


/**
 * @brief Add noises to a message.
 * @note This function is called by the proxy.
 * 
 * @param msg The message to add noise to.
 * @param nb_errors The number of errors to add.
 */
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

