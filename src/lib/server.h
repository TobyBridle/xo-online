#ifndef NOUGHTS_CROSSES_SERVER_H
#define NOUGHTS_CROSSES_SERVER_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#define TCP 0
#define loop while (1)

typedef struct {
  int socket;
  int client_id;
  char *client_name;
  struct sockaddr_in addr;
  enum { NOUGHT, CROSS, SPECTATOR } player_type;
} client_t;

typedef struct {
  client_t *clients;
  int count;
} clients_t;

typedef struct {
  int socket;
  short port;
  clients_t conns;
} server_t;

server_t server_init(short port);

// sever_init() returns a server_t struct with a bound socket and port
int server_bind(server_t *server);
int server_listen(server_t *server);

void server_serve(server_t *server);

client_t server_accept(server_t *server);
int server_handle(server_t *server, client_t *client, char *message);

int server_unbind(server_t *server);

// handle_sock_error() takes in the global errno and uses a switch case
void handle_sock_error(int err) {
  switch (err) {
  case EACCES:
    fprintf(stderr, "Permission to create a socket of the specified type "
                    "and/or to bind to the specified port is denied.\n");
    break;
  case EADDRINUSE:
    fprintf(stderr, "The given address is already in use.\n");
    break;
  case EADDRNOTAVAIL:
    fprintf(stderr,
            "The specified address is not available from the local machine.\n");
    break;
  case EAFNOSUPPORT:
    fprintf(
        stderr,
        "Addresses in the specified family cannot be used with this socket.\n");
    break;
  case EINVAL:
    fprintf(stderr, "The socket is already bound to an address.\n");
    break;
  case EMFILE:
    fprintf(stderr,
            "The process already has the maximum number of files open.\n");
    break;
  case ENFILE:
    fprintf(stderr, "The system limit on the total number of open files has "
                    "been reached.\n");
    break;
  case ENOBUFS:
    fprintf(stderr, "Insufficient resources were available in the system to "
                    "perform the operation.\n");
    break;
  case ENOMEM:
    fprintf(stderr,
            "Insufficient memory was available to fulfill the request.\n");
    break;
  case EPROTONOSUPPORT:
    fprintf(stderr, "The protocol type or the specified protocol is not "
                    "supported within this domain.\n");
    break;
  case EPROTOTYPE:
    fprintf(stderr, "The protocol type is the wrong type for this socket.\n");
    break;
  case ESOCKTNOSUPPORT:
    fprintf(stderr,
            "The socket type is not supported in this address family.\n");
    break;
  default:
    fprintf(stderr, "Unknown socket error: %d\n", err);
    break;
  }
}

extern const int MAX_CLIENTS;
#endif
