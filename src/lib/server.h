#ifndef NOUGHTS_CROSSES_SERVER_H
#define NOUGHTS_CROSSES_SERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define TCP 0
#define loop while (1)

typedef struct {
  int socket;
  int client_id; // This corresponds to the index in the server->conns->clients
                 // array
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

typedef struct {
  int count;          // We use this to set the client_id of our players
  client_t clients[]; // Static array of size MAX_CLIENTS
} clients_t;

typedef struct {
  int socket;
  short port;
  clients_t conns;
} server_t;

/**
 * @brief Creates a new server instance
 *
 * @param port
 * @return server_t
 */
server_t server_init(short port);

// Helpers used by the server_init function
int server_bind(server_t *server);
int server_listen(server_t *server);

/**
 * @brief Registers the new threads and starts to listen to input.
 *
 * @param server
 */
void server_start(server_t *server);

/**
 * @brief Accepts new client connections and adds them to the server
 *
 * @param server
 */
void server_serve(server_t *server);

// Helpers used by the server_serve function
client_t server_accept(server_t *server);
int server_handle(server_t *server, client_t *client, char *message);

/**
 * @brief Free any memory allocated and close the socket(s).
 *
 * @param server
 * @return
 */
int server_unbind(server_t *server);

/**
 * @brief Used for interacting with the server from the parent process
 *
 * @param server
 * @param subprocess
 */
void handle_commands(server_t *server, pid_t subprocess);

/**
 * @brief Handle the errno provided by the socket failures
 *
 * @param err
 */
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
#endif
