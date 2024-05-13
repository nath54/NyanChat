
/*

Code suivant à adapter:
    il y a base recepteur TCP

*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define INVALID_SOCKET -1
#define SOCKET_ERROR -1


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


int main(int argc, char **argv)
{
    // descripteur de socket
    SOCKET sockfd;
    // structure d'adresse qui contiendra les
    SOCKADDR_IN dest = {0};

    // parametres reseaux du destinataire

    // verification du nombre d'arguments sur la ligne de commande
    if(argc != 4) {
        printf("Usage : %s @dest num_port chaine_a_envoyer\n", argv[0]);
        exit(-1);
    }

    char* addr = argv[1];
    int port = atoi(argv[2]);
    char* msg =  argv[3];
    unsigned int tmsg = strlen(msg);

    // creation de la socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sockfd == INVALID_SOCKET){
        fprintf(stderr, "Erreur lors de la création du socket!\n");
        exit(errno);
    }

    // initialisation de la structure d'adresse du destinataire :

    // famille d'adresse
    dest.sin_family = AF_INET;

    // adresse IPv4 du destinataire
    inet_aton(addr , &(dest.sin_addr));

    // port du destinataire
    dest.sin_port = htons(port);

    //
    if(connect(sockfd,(SOCKADDR *) &dest, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        exit(errno);
    }


    // envoi de la chaine
    if(sendto(sockfd, msg, tmsg, 0, (SOCKADDR *)&dest, sizeof(dest)) == -1)
      {
        perror("erreur a l'appel de la fonction sendto -> ");
        exit(errno);
      }

    // fermeture de la socket
    close(sockfd);

    return 0;
}
