#include "lib/client.h"

#define PORT 80
#define loop while (1)

int main() {
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
