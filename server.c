#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define ADDRESS "127.0.0.1"

struct Header {
  char method[8];     // e.g., "GET", "POST"
  char path[128];     // e.g., "/index.html"
  char protocol[16];  // e.g., "HTTP/1.1"
};

void parse_request(const char* request_str, struct Header* request) {
  sscanf(request_str, "%s %s %s", request->method, request->path,
         request->protocol);
}

int main(int argc, char** argv) {
  struct in_addr addr;
  struct sockaddr_in addr_in, addr_peer;
  socklen_t addr_peer_size = sizeof(addr_peer);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(PORT);

  inet_pton(AF_INET, ADDRESS, &addr);
  addr_in.sin_addr = addr;

  if (bind(sock, (struct sockaddr*)&addr_in, sizeof(addr_in)) == -1) {
    perror("Socket binding failed");
    exit(EXIT_FAILURE);
  }

  if (listen(sock, 3) == -1) {
    perror("Listening failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on %s:%d\n", ADDRESS, PORT);

  int sock_in = accept(sock, (struct sockaddr*)&addr_peer, &addr_peer_size);
  if (sock_in == -1) {
    perror("Accepting failed");
    exit(EXIT_FAILURE);
  }

  char buffer[1024] = {0};
  read(sock_in, buffer, 1024);
  printf("Received request:\n%s\n", buffer);
  struct Header request;
  parse_request(buffer, &request);

  switch (request.method) {
    case "GET":
      /* code */
      break;

    default:
      break;
  }

  close(sock_in);
  close(sock);

  return 0;
}