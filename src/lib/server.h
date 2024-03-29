#ifndef NOUGHTS_CROSSES_SERVER_H
#define NOUGHTS_CROSSES_SERVER_H

#include "client.h"
#include "utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
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

#define HEADER_VERB (game_count > 1 || game_count == 0 ? "are" : "is")

#define HEADER_GAME (game_count > 1 || game_count == 0 ? "games" : "game")

const int MAX_CLIENTS = 1000;

enum SERVER_STATE { ACCEPTING, NOT_ACCEPTING };

typedef struct {
  int socket;
  short port;
  HashMap clients;
  enum SERVER_STATE state;
  LinkedList *games;
  unsigned long current_game_hash;
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
client_t *server_accept(server_t *server);

void handle_client_name_set(client_t *client, char *buf);
void handle_game_create(server_t *server, client_t *client);
int handle_game_join(server_t *server, client_t *client);
void handle_game_unbind(server_t *server, client_t *client);
void handle_client_disconnect(server_t *server, client_t *client,
                              int client_id);

int render_games_page(server_t *server, client_t *client);

void smart_broadcast(client_t **clients, size_t amount, char *message,
                     size_t len);

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
unsigned long hash_games_list(LinkedList *games);
#endif
