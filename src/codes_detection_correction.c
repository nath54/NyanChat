#include "codes_detection_correction.h"


// Fonction pour détecter une erreur dans le message
//  Renvoie 0 si pas d'erreurs détectées,
//  sinon une valeur positive qui peut par exemple indiquer si possibilité de
//  corriger l'erreur ou non, jsp si c'est possible 
int code_detect_error(Message* msg){

    (void)msg;
    
    // TODO: compléter cette fonction

    return 0;
}


// Fonction qui corrige directement l'erreur dans msg
//  Renvoie 0 si tout s'est bien passé
//  Sinon, renvoie -1
int code_correct_error(Message* msg){

    (void)msg;

    // TODO: compléter cette fonction

    return 0;
}



// Fonction qui va bruiter un message
//   (appelée par le proxy)
// Ajoute nb_errors erreurs dans le Message
void code_add_noise_to_msg(Message* msg, int nb_errors){
    
    (void)msg;
    (void)nb_errors;

    // TODO: compléter cette fonction

}

