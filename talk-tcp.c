#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHK(op)                                                               \
    do                                                                        \
    {                                                                         \
        if ((op) == -1)                                                       \
            perror(#op);                                                      \
    } while (0)

int make_connection(char *addr, int port)
{
    struct sockaddr_in server;
    // initialisation de la structure d'adresse du destinataire
    server.sin_family = AF_INET; // famille d'adresse
    inet_pton(AF_INET, addr, &server.sin_addr); // adresse IPv4 du destinataire
    server.sin_port = htons(port);
    memset(&server.sin_zero, '\0', 8);

    int sockfd;
    CHK(sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

    // connexion au destinataire, pas besoin de bind
    sleep(10);
    CHK(connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)));
    return sockfd;
}

int create_socket(int port)
{
    struct sockaddr_in my_addr; // structure d'adresse du recepteur

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(my_addr.sin_addr));

    // association de la socket et des param reseaux du recepteur
    int sockfd;
    CHK(sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    // association de la socket et des param reseaux du recepteur
    CHK(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)));
    // attente de connexion avec une file de 2 clients maximum
    CHK(listen(sockfd, 2));
    return sockfd;
}

/**
 * Update chat output when receiving a message
*/
void receiver(int my_port, struct sockaddr_in* from)
{
    int sock_fd = create_socket(my_port);

    socklen_t fromlen = sizeof(struct sockaddr_in);
    // attente de connexion d'un client
    int sock_cl;
    CHK(sock_cl = accept(sock_fd, (struct sockaddr *)&from, &fromlen));

    char buf[1024];
    int n;
    while (1) {
        CHK(n = recv(sock_cl, buf, sizeof(buf), 0));
        if (n == 0) {
            printf("Connection closed\n");
            CHK(close(sock_cl));
            break;
        }
        buf[n] = '\0';
        printf("[Received] : %s", buf);
    }
}

/**
 * Read a message from the standard input and 
 * send a message to the server chat
*/
void sender(char *addr, int port)
{
    int sock_serv_fd = make_connection(addr, port);

    char buf[1024];
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        if (strcmp(buf, "exit\n") == 0) {
            CHK(close(sock_serv_fd));
            break;
        }
        CHK(send(sock_serv_fd, buf, strlen(buf), 0));
    }
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        printf("Usage : %s @dest num_port my_port \n", argv[0]);
        exit(1);
    }
    char *addr = argv[1];
    int port = atoi(argv[2]);
    int my_port = atoi(argv[3]);

    // initialisation de la structure d'adresse de l'expediteur
    struct sockaddr_in from;
    from.sin_family = AF_INET;
    from.sin_port = htons(0);
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    
    switch (fork()) {
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            receiver(my_port, &from);
            exit(0);
        default:
            sender(addr, port);
            break;
    }

    return 0;
}