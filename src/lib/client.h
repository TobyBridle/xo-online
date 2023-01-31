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
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define TCP 0
#define RECONNECT_INTERVAL                                                     \
  1 // This is the amount of seconds inbetween reconnect attempts

const uint8_t MAX_CLIENT_NAME_LENGTH = 24;

#ifndef NOUGHTS_CROSSES_CLIENT_T
#define NOUGHTS_CROSSES_CLIENT_T
typedef struct {
  int socket;
  int client_id; // This corresponds to the index in the server->conns->clients
                 // array and is -1 if the client has not yet been registered by
                 // the server
  const char *client_name; // This is the display name
                           // of the user, which we will
                           // show other players
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

#ifndef NEWLINE_KEY
#define NEWLINE_KEY 13
#endif

#ifndef CTRL_C_KEY
#define CTRL_C_KEY 3
#endif

static struct termios restore;
static struct termios noughts_crosses_term;

void hide_term_cursor();
void show_term_cursor();

void enable_raw_term();
void disable_raw_term();

/* ------------------------------------------------------------------------ */

client_t *client_init();
int client_connect(int server_fd, client_t *client);
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

void print_buffer(char *buf);
#endif
