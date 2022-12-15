#ifndef NOUGHTS_CROSSES_SERVER_H
#define NOUGHTS_CROSSES_SERVER_H

#include "client.h"
#include "utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
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

const int MAX_CLIENTS = 1000;

enum SERVER_STATE { ACCEPTING, NOT_ACCEPTING };

typedef struct CLIENTS_T {
  int count; // We use this to set the client_id of our players
  int max_clients;
  client_t clients[MAX_CLIENTS]; // Static array of size MAX_CLIENTS
} clients_t;

typedef struct {
  int socket;
  short port;
  /* int pipefd[2]; */
  enum SERVER_STATE state;
  clients_t conns;
} server_t;

/* ------------------------------------------------------------------------ */

/**
 * @brief Creates a new server instance
 *
 * @param port
 * @return server_t
 */
server_t *server_init(short port);

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
/* void handle_commands(server_t *server, pid_t subprocess); */
#endif
