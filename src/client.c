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

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define BUFFER_SIZE 1024

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int main(int argc, char* argv[]) {
  // Socket descriptor
  SOCKET sockfd;
  // Server address structure
  SOCKADDR_IN dest = {0};
  // Buffer for received data
  char buffer[BUFFER_SIZE];
  // File descriptor for standard input
  int stdin_fd = fileno(stdin);

  // Check arguments
  if (argc != 3) {
    printf("Usage: %s ip_proxy port_proxy\n", argv[0]);
    exit(-1);
  }

  char* ip_proxy = argv[1];
  int port_proxy = atoi(argv[2]);

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == INVALID_SOCKET) {
    perror("socket");
    exit(errno);
  }

  // Initialize server address
  dest.sin_family = AF_INET;
  inet_aton(ip_proxy, &dest.sin_addr);
  dest.sin_port = htons(port_proxy);

  // Connect to server
  if (connect(sockfd, (SOCKADDR*)&dest, sizeof(SOCKADDR)) == SOCKET_ERROR) {
    perror("connect");
    exit(errno);
  }

  // Create poll structure
  struct pollfd fds[2];
  fds[0].fd = sockfd;
  fds[0].events = POLLIN; // Interested in readability on socket
  fds[1].fd = stdin_fd;
  fds[1].events = POLLIN; // Interested in readability on standard input

  while (1) {
    int ret = poll(fds, 2, -1); // Wait indefinitely
    if (ret == -1) {
      perror("poll");
      break;
    } else if (ret == 0) {
      // Timeout (shouldn't happen here)
      continue;
    }

    // Check for activity on socket
    if (fds[0].revents & POLLIN) {
      int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
      if (bytes_received == 0) {
        printf("Server closed connection\n");
        break;
      } else if (bytes_received == -1) {
        perror("recv");
        break;
      }

      // Process received data
      printf("Received from server: %s\n", buffer);
    }

    // Check for user input
    if (fds[1].revents & POLLIN) {
      // Read message from standard input
      int bytes_read = read(stdin_fd, buffer, BUFFER_SIZE);
      if (bytes_read == 0) {
        printf("User closed input\n");
        break;
      } else if (bytes_read == -1) {
        perror("read");
        break;
      }

      // Send message to server
      buffer[bytes_read - 1] = '\0'; // Replace newline with null terminator
      int bytes_sent = send(sockfd, buffer, bytes_read, 0);

      printf("%d bytes sent!\n", bytes_sent);

      if (bytes_sent == -1) {
        perror("send");
        break;
      }
    }
  }

  // Close socket
  close(sockfd);

  return 0;
}
