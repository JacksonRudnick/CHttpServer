#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8079
#define ADDRESS "127.0.0.1"

struct Header {
  char method[8];     // e.g., "GET", "POST"
  char path[128];     // e.g., "/index.html"
  char protocol[16];  // e.g., "HTTP/1.1"
};

struct Response {
  char protocol[16];     // e.g., "HTTP/1.1"
  int status_code;       // e.g., 200, 404
  char status_text[64];  // e.g., "OK", "Not Found"
  char headers[256];     // Response headers
  char body[1024];       // Response body
};

void parse_request(const char* request_str, struct Header* request) {
  sscanf(request_str, "%s %s %s", request->method, request->path,
         request->protocol);
}

enum ResponseStatus { OK = 200, NOT_FOUND = 404, METHOD_NOT_ALLOWED = 405 };

void build_response(struct Response* response, enum ResponseStatus status,
                    const char* status_text, const char* headers,
                    const char* body_content) {
  strcpy(response->protocol, "HTTP/1.1");
  response->status_code = status;
  strcpy(response->status_text, status_text);
  strcpy(response->headers, headers);
  strcpy(response->body, body_content);
}

void read_html_file(const char* filename, char* buffer, size_t buffer_size) {
  FILE* file = fopen(filename, "r");
  if (file) {
    fread(buffer, 1, buffer_size - 1, file);
    buffer[buffer_size - 1] = '\0';  // Ensure null-termination
    fclose(file);
  } else {
    perror("Failed to open file");
    buffer[0] = '\0';  // Empty body on failure
  }
}

void serialize_response(const struct Response* response, char* buffer,
                        size_t buffer_size) {
  snprintf(buffer, buffer_size,
           "%s %d %s\r\n"
           "%s"
           "Content-Length: %zu\r\n"
           "\r\n"
           "%s",
           response->protocol, response->status_code, response->status_text,
           response->headers, strlen(response->body), response->body);
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

  if (strcmp(request.method, "GET") == 0) {
    struct Response* response = malloc(sizeof(struct Response));

    char response_body_buffer[1024];
    read_html_file("index.html", response_body_buffer,
                   sizeof(response_body_buffer));

    build_response(response, OK, "OK", "Content-Type: text/html\r\n",
                   response_body_buffer);

    char response_buffer[4096];
    serialize_response(response, response_buffer, sizeof(response_buffer));
    send(sock_in, response_buffer, strlen(response_buffer), 0);

    printf("Sending response:\n%s\n", response_buffer);

    free(response);
  } else {
    const char* http_response =
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    send(sock_in, http_response, strlen(http_response), 0);
  }

  close(sock_in);
  close(sock);

  return 0;
}