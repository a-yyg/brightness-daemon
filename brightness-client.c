//
// messeger.c
// Sends messages through socket to the brightness-daemon.
// Author: ayyg
//
// Possible messages:
// - "up" -- increase brightness
// - "down" -- decrease brightness
// - "reset" -- reset brightness to default
// - "max" -- set brightness to maximum
// - "min" -- set brightness to minimum

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  // Boilerplate
  if (argc < 4) {
    fprintf(stderr, "usage %s hostname port message\n", argv[0]);
    exit(0);
  }

  // Create socket
  // AF_INET: IPv4
  // SOCK_STREAM: TCP
  // 0: default protocol
  // socket(int domain, int type, int protocol)
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  // Get hostname
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  // Set port number
  portno = atoi(argv[2]);

  // Set server address
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  // Connect to server
  // connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    exit(1);
  }

  // Send message
  // send(int sockfd, const void *msg, size_t len, int flags)
  n = send(sockfd, argv[3], strlen(argv[3]), 0);
  if (n < 0) {
    perror("ERROR writing to socket");
    exit(1);
  }

  // Close socket
  close(sockfd);

  return 0;
}
