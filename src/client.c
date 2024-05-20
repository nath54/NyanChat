#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// #include "../include/tcp_connection.h"
#include "../include/client.h"


void on_stdin_client(TcpConnection* con,
                    char msg[T_MSG_MAX],
                    size_t msg_len,
                    void* custom_args)
{
    (void)custom_args;

    int bytes_sent = send(con->sockfd, msg, msg_len, 0);

    printf("%d bytes sent!\n", bytes_sent);

    if (bytes_sent == -1) {
        perror("send");
        con->end_connection = true;
    }
}


void on_msg_client(TcpConnection* con, SOCKET sock, 
                   Message msg, size_t msg_len,
                   void* custom_args){
    (void)con;
    (void)sock;
    (void)msg_len;
    (void)custom_args;
    printf("Message received: %s\n", msg.msg);

}



int main(int argc, char* argv[]) {
    
    TcpConnection con;    

    // Check arguments
    if (argc != 3) {
        printf("Usage: %s ip_proxy port_proxy\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* ip_proxy = argv[1];
    int port_proxy = atoi(argv[2]);

    tcp_connection_client_init(&con, ip_proxy, port_proxy, -1);

    tcp_connection_mainloop(&con,
                            on_msg_client, NULL,
                            on_stdin_client, NULL);

    tcp_connection_close(&con);

    return 0;
}
