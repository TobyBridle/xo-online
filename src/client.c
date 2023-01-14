#include "lib/client.h"

#define PORT 80
#define loop while (1)

int main() {
  client_t *client = client_init();
  char buffer[1024];
  struct pollfd fds[1];
  fds[0].fd = client->socket;
  fds[0].events = POLL_IN;

  int client_id = -1;
  enable_raw_term();

  char c;

  while (client->socket == fds[0].fd) {
    // Check if we have a connection
    // The first thing we will receive is the client ID

    int poll_status = poll(fds, 1, 100);
    char peek_buf[1] = {0};
    if (poll_status > 0) {
      int read_status = recv(client->socket, peek_buf, 1, MSG_PEEK);
      if (fds[0].revents & POLL_IN) {
        // We have received a message
        if (read_status == -1) {
          disable_raw_term();
          handle_sock_error(errno);
          exit(1);
        } else if (read_status == 0) {
          // The server has disconnected
          printf("\x1b[31;1mServer has disconnected\x1b[0m\r\n");
          break;
        }

        recv(client->socket, buffer, 1024, 0);
        if (client_id == -1) {
          // We have received the client ID
          // We need to use strtol to convert the string to an integer
          /* client_id = deserialize_int(buffer); */
          client_id = strtol(buffer, NULL, 10);

          printf("\x1b[32;1mConnected to server as client %s\x1b[0m\r\n",
                 buffer);
        }
        printf("Buffer: %s\r\n", buffer);
      } else if (fds[0].revents & POLL_ERR) {
        printf("\x1b[31;1mError occurred\x1b[0m\r\n");
      }
    }

    // Check if input
    if (read(STDIN_FILENO, &c, 1) == 1) {
      switch (c) {
      case QUIT_KEY:
      case 3:
        fds[0].fd = -1; // NOTE: This will prevent polls from occuring and will
                        // break our loop
        break;
      }
    }
  }
  disable_raw_term();
  client_disconnect(client);
  return 0;
}

void enable_raw_term() {
  tcgetattr(STDIN_FILENO, &restore);
  atexit(disable_raw_term);

  noughts_crosses_term = restore;

  noughts_crosses_term.c_iflag &= ~(BRKINT | ICRNL | IXON);
  noughts_crosses_term.c_oflag &= ~(OPOST);
  // ECHO - Print each key to terminal
  // ICANON - Read on a byte-by-byte basis, rather than by line
  // ISIG - Read control characters as their bytes (e.g CTRL+C is 3)
  noughts_crosses_term.c_lflag &= ~(ECHO | ICANON | ISIG);

  noughts_crosses_term.c_cc[VMIN] = 0;
  noughts_crosses_term.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &noughts_crosses_term);
}

void disable_raw_term() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &restore); }

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
