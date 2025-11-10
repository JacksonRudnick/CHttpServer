#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define ADDRESS "127.0.0.1"

int main(int argc, char** argv) {
  struct in_addr* addr;
  socklen_t* addr_peer_size;
  struct sockaddr_in *addr_in, *addr_peer;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  addr = malloc(sizeof(addr));
  addr_in = malloc(sizeof(addr_in));

  addr_in->sin_family = AF_INET;
  addr_in->sin_port = PORT;

  inet_pton(AF_INET, ADDRESS, addr);
  addr_in->sin_addr = *addr;

  if (connect(sock, addr_in, sizeof(*addr_in)) == -1) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  send(sock, "Hello, Server!", 14, 0);

  free(addr);
  free(addr_in);
  free(addr_peer);
  free(addr_peer_size);

  close(sock);

  return 0;
}