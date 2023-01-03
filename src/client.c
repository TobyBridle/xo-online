#include "lib/client.h"

#define PORT 80
#define loop while (1)

int main(int argc, char *argv[]) {
  client_t *client = client_init();
  char buffer[1024];
  loop {
    // Check if we have a connection
    if (read(client->socket, buffer, 1024) == 0) {
      break;
    }
  }
  client_disconnect(client);
  return 0;
}

client_t *client_init() {
  client_t *client = (client_t *)malloc(sizeof(client_t));
  int socket_fd = socket(AF_INET, SOCK_STREAM, TCP);
  if (socket_fd == -1) {
    handle_sock_error(errno);
    exit(1);
  }
  printf("\x1b[32;1mSocket created successfully\x1b[0m\n");

  printf("\x1b[33;1mAttempting to connect to server\x1b[0m\n");
  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_addr = INADDR_ANY,
                                    .sin_len = sizeof(server_addr),
                                    .sin_port = htons(PORT)};
  int connect_status =
      connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (connect_status == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  client->socket = socket_fd;
  client->addr = server_addr;

  // NOTE: THESE ARE DECIDED BY THE SERVER
  client->client_id = -1;
  client->player_type = SPECTATOR;

  printf("\x1b[32;1mSuccessfully sent username to server\x1b[0m\n");
  return client;
}

void client_disconnect(client_t *client) {
  printf("\x1b[33;1mAttempting to disconnect from server\x1b[0m\n");
  int close_status = close(client->socket);
  if (close_status == -1) {
    handle_sock_error(errno);
    free(client);
    exit(1);
  }
  printf("\x1b[32;1mSuccessfully disconnected from server\x1b[0m\n");
  free(client);
}

#ifndef HANDLE_SOCK_ERROR_FN
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

