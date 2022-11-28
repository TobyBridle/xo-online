#ifndef NOUGHTS_CROSSES_CLIENT_H
#define NOUGHTS_CROSSES_CLIENT_H
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/signal.h>
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
    perror("\x1b[31;1mPermission to create a socket of the specified type "
           "and/or to bind to the specified port is denied.\x1b[0m\n");
    break;
  case EADDRINUSE:
    perror("\x1b[31;1mThe given address is already in use.\x1b[0m\n");
    break;
  case EADDRNOTAVAIL:
    perror("\x1b[31;1mThe specified address is not available from the local "
           "machine.\x1b[0m\n");
    break;
  case EAFNOSUPPORT:
    perror("Addresses in the specified family cannot be used with this "
           "socket.\x1b[0m\n");
    break;
  case EINVAL:
    perror("\x1b[31;1mThe socket is already bound to an address.\x1b[0m\n");
    break;
  case EMFILE:
    perror("\x1b[31;1mThe process already has the maximum number of files "
           "open.\x1b[0m\n");
    break;
  case ENFILE:
    perror("\x1b[31;1mThe system limit on the total number of open files has "
           "been reached.\x1b[0m\n");
    break;
  case ENOBUFS:
    perror("\x1b[31;1mInsufficient resources were available in the system to "
           "perform the operation.\x1b[0m\n");
    break;
  case ENOMEM:
    perror("\x1b[31;1mInsufficient memory was available to fulfill the "
           "request.\x1b[0m\n");
    break;
  case EPROTONOSUPPORT:
    perror("\x1b[31;1mThe protocol type or the specified protocol is not "
           "supported within this domain.\x1b[0m\n");
    break;
  case EPROTOTYPE:
    perror("\x1b[31;1mThe protocol type is the wrong type for this "
           "socket.\x1b[0m\n");
    break;
  case ESOCKTNOSUPPORT:
    perror("\x1b[31;1mThe socket type is not supported in this address "
           "family.\x1b[0m\n");
    break;
  case ECONNREFUSED:
    perror("\x1b[31;1mNo one listening on the remote address.\x1b[0m\n");
    break;
  default:
    fprintf(stderr, "Unknown error: %d\x1b[0m\n", err);
    break;
  }
}
#endif
