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
#endif
