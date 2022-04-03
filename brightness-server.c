//
// brightness-daemon
// Controls brightness of a monitor through xrandr by receiving messages
// from a socket.
// Author: ayyg
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SLEEP_TIME 100000
#define MAX_BUFFER_SIZE 1024
#define STEP 0.05

float get_brightness();
void set_brightness(float brightness);

void daemonize();

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s <host> <port> [-d]\n", argv[0]);
    printf("\t-d: run as a daemon\n");
    exit(1);
  }

  int port = atoi(argv[2]);

  if (argc == 4 && strcmp(argv[3], "-d") == 0) {
    daemonize();
  }

  float brightness = 0;
  float max_brightness = 1;

  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(1);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(port);

  if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &sockfd, sizeof(int)) < 0) {
    perror("ERROR setting socket options");
    exit(1);
  }

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR on binding");
    exit(1);
  }

  //int sock = socket(AF_INET, SOCK_STREAM, 0);
  //struct sockaddr_in addr;
  //addr.sin_family = AF_INET;
  //addr.sin_port = htons(port);
  //addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  while (1) {
    // Get current brightness
    double cur_brightness = get_brightness();

    // Listen to host:port for messages

    if (listen(sockfd, 5) < 0) {
      perror("ERROR on listen");
      exit(1);
    }

    // Accept connection
    int client = accept(sockfd, NULL, NULL);
    if (client < 0) {
      perror("accept");
      exit(1);
    }

    // Read message
    char buffer[MAX_BUFFER_SIZE];
    int len = read(client, buffer, sizeof(buffer));
    if (len < 0) {
      perror("read");
      exit(1);
    }

    buffer[len] = '\0';

    close(client);


    // Parse message
    if (strcmp(buffer, "up") == 0) {
      // Increase brightness
      brightness = cur_brightness + STEP;
      if (brightness > max_brightness) {
        brightness = max_brightness;
      }
    } else if (strcmp(buffer, "down") == 0) {
      // Decrease brightness
      brightness = cur_brightness - STEP;
      if (brightness < 0) {
        brightness = 0;
      }
    } else if (strcmp(buffer, "reset") == 0) {
      // Reset brightness
      brightness = 0.5;
    } else if (strcmp(buffer, "max") == 0) {
      // Set to max brightness
      brightness = max_brightness;
    } else if (strcmp(buffer, "min") == 0) {
      // Set to min brightness
      brightness = 0;
    } else {
      // Unknown message
      continue;
    }

    // Set brightness
    set_brightness(brightness);

    usleep(SLEEP_TIME);
  }

  // Close connection
  close(sockfd);

  return 0;
}

float get_brightness() {
  FILE *fp;
  float brightness = 0;
  char command[256];
  sprintf(command, "xrandr --verbose | grep 'Brightness' | cut -d ' ' -f 2");
  fp = popen(command, "r");
  fscanf(fp, "%f", &brightness);
  pclose(fp);
  return brightness;
}

void set_brightness(float brightness) {
  FILE *fp;
  char command[256];
  sprintf(command, "xrandr --output eDP-1 --brightness %f", brightness);
  fp = popen(command, "r");
  pclose(fp);
}

void daemonize() {
  pid_t pid, sid;

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  sid = setsid();

  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

