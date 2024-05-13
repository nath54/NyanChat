#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#define TAILLE_BUF 1024


/*

Code suivant à adapter:
    il y a base recepteur TCP

*/

int main(int argc, char **argv)
{
    // descripteur de socket
    SOCKET sockfd;
    // espace necessaire pour stocker le message recu
    char buf[TAILLE_BUF];

    // taille d'une structure sockaddr_in utile pour la fonction recvfrom
    socklen_t fromlen = sizeof(SOCKADDR_IN); 

    // structure d'adresse qui contiendra les param reseaux du recepteur
    SOCKADDR_IN my_addr;
    // structure d'adresse qui contiendra les param reseaux de l'expediteur
    SOCKADDR_IN client;

    SOCKET acc_sockfd;

    // verification du nombre d'arguments sur la ligne de commande
    if(argc != 2)
    {
        printf("Usage: %s port_local\n", argv[0]);
        exit(-1);
    }

    int port = atoi(argv[1]);

    // creation de la socket

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd == INVALID_SOCKET){
        fprintf(stderr, "Erreur lors de la création du socket!\n");
        exit(errno);
    }

    // initialisation de la structure d'adresse du recepteur (pg local)

    // nous sommes un serveur, nous acceptons n'importe quelle adresse
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // famille d'adresse
    my_addr.sin_family = AF_INET;

    // recuperation du port du recepteur
    my_addr.sin_port = htons(port);

    // adresse IPv4 du recepteur
    inet_aton("127.0.0.1", &(my_addr.sin_addr));

    // association de la socket et des param reseaux du recepteur
    if(bind(sockfd, (SOCKADDR *)&my_addr, sizeof(my_addr)) != 0)
      {
        perror("erreur lors de l'appel a bind -> ");
        exit(errno);
      }

    if(listen(sockfd, 5) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }

    acc_sockfd = accept(sockfd, (SOCKADDR *)&my_addr, &fromlen);

    // reception de la chaine de caracteres
    if( recvfrom(acc_sockfd, buf, TAILLE_BUF, 0,
                    (SOCKADDR *)&my_addr, &fromlen ) == -1)
    {
        perror("erreur de reception -> ");
        exit(errno);
    }

    // affichage de la chaine de caracteres recue
    printf("received from socket : \"%s\"\n", buf);

    // fermeture de la socket
    close(acc_sockfd);
    close(sockfd);

    return 0;
}
