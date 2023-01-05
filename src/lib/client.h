#ifndef NOUGHTS_CROSSES_CLIENT_H
#define NOUGHTS_CROSSES_CLIENT_H
#include "utils.h"
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define TCP 0

#ifndef NOUGHTS_CROSSES_CLIENT_T
#define NOUGHTS_CROSSES_CLIENT_T
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
#endif

/* ------------------------------------------------------------------------ */

#ifndef QUIT_KEY
#define QUIT_KEY 'q'
#endif

static struct termios restore;
static struct termios noughts_crosses_term;

void enable_raw_term();
void disable_raw_term();

/* ------------------------------------------------------------------------ */

client_t *client_init();
void client_disconnect(client_t *client);

#ifndef HANDLE_SOCK_ERROR_FN
#define HANDLE_SOCK_ERROR_FN
/**
 * @brief Handle the errno provided by the socket failures
 *
 * @param err
 */
void handle_sock_error(int err);
#endif
#endif
