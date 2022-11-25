#ifndef NOUGHTS_CROSSES_CLIENT_H
#define NOUGHTS_CROSSES_CLIENT_H
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <unistd.h>

#define TCP 0

typedef struct {
  int socket;
  int client_id; // This corresponds to the index in the server->conns->clients
                 // array and is -1 if the client has not yet been registered by
                 // the server
  char *client_name; // This is the display name of the user, which we will show
                     // other players
  struct sockaddr_in addr;
  enum {
    NOUGHT,
    CROSS,
    SPECTATOR
  } player_type; // Spectator by default, so that the user cannot interact with
                 // any games
} client_t;

/* ------------------------------------------------------------------------ */

client_t *client_init();
void client_disconnect(client_t *client);
/**
 * @brief Handle the errno provided by the socket failures
 *
 * @param err
 */
void handle_sock_error(int err) {
  switch (err) {
  case EACCES:
    perror("Permission to create a socket of the specified type "
           "and/or to bind to the specified port is denied.\n");
    break;
  case EADDRINUSE:
    perror("The given address is already in use.\n");
    break;
  case EADDRNOTAVAIL:
    perror("The specified address is not available from the local machine.\n");
    break;
  case EAFNOSUPPORT:
    perror(
        "Addresses in the specified family cannot be used with this socket.\n");
    break;
  case EINVAL:
    perror("The socket is already bound to an address.\n");
    break;
  case EMFILE:
    perror("The process already has the maximum number of files open.\n");
    break;
  case ENFILE:
    perror("The system limit on the total number of open files has "
           "been reached.\n");
    break;
  case ENOBUFS:
    perror("Insufficient resources were available in the system to "
           "perform the operation.\n");
    break;
  case ENOMEM:
    perror("Insufficient memory was available to fulfill the request.\n");
    break;
  case EPROTONOSUPPORT:
    perror("The protocol type or the specified protocol is not "
           "supported within this domain.\n");
    break;
  case EPROTOTYPE:
    perror("The protocol type is the wrong type for this socket.\n");
    break;
  case ESOCKTNOSUPPORT:
    perror("The socket type is not supported in this address family.\n");
    break;
  case ECONNREFUSED:
    perror("No one listening on the remote address.\n");
    break;
  default:
    fprintf(stderr, "Unknown error: %d\n", err);
    break;
  }
}
#endif
