#include "codes_detection_correction.h"


// Function to detect an error in the message
// Returns 0 if no errors are detected,
// otherwise a positive value that can for example indicate whether or not it is possible to
// correct the error, idk if it's possible
int code_detect_error(Message* msg){

    (void)msg;
    
    // TODO: compléter cette fonction

    return 0;
}


// Function that directly corrects the error in msg
// Returns 0 if everything went well
// Otherwise, returns -1
int code_correct_error(Message* msg){

    (void)msg;

    // TODO: compléter cette fonction

    return 0;
}



// Function that will add noise to a message
//   (called by the proxy)
// Adds nb_errors errors to the Message
void code_add_noise_to_msg(Message* msg, int nb_errors){
    
    (void)msg;
    (void)nb_errors;

    // TODO: compléter cette fonction

}

