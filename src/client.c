#include "lib/client.h"

#define PORT 80
#define loop while (1)

int main() {
  client_t *client = client_init();
  // Initially, requires_username will be TRUE when the client_id is first
  // sent through. It'll then remain as true until the enter key is pressed when
  // entering the name.
  BOOL requires_username;
  uint8_t client_name_length = 0;

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

    int poll_interval = client_id == -1 ? RECONNECT_INTERVAL * 1000 : 100;
    int poll_status = poll(fds, 1, poll_interval);
    char peek_buf[7] = {0};
    if (poll_status > 0) {
      int read_status = recv(client->socket, peek_buf, 7, MSG_PEEK);

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
          client->client_id = client_id;
          requires_username = TRUE;
        }
        print_buffer(buffer);
      } else if (fds[0].revents & POLL_ERR) {
        printf("\x1b[31;1mError occurred\x1b[0m\r\n");
      }
    } else if (poll_status == 0 && client_id == -1) {
      fprintf(stderr,
              "\x1b[31;1mCould not connect to Server. There may be too many "
              "connections\r\nRetrying in %ds\x1b[0;0m\r\n",
              RECONNECT_INTERVAL);
      client_connect(fds[0].fd, client);
    }

    // Check if input
    int input_status = read(STDIN_FILENO, &c, 1);

    if (requires_username) {
      if (input_status < 1) {
        printf("\033[s");        // save cursor position
        printf("\033[%d;0H", 7); // move cursor to y-coordinate 7
        printf("\033[;1mPlease Enter your Name:\x1b[;0m\r\n");
      }
      // We have had some input and after we have
      // finished our conditions we will have to flush the output.
      else if (c == CTRL_C_KEY) {
        fds[0].fd = -1;
        break;
      } else if (c != NEWLINE_KEY &&
                 client_name_length < MAX_CLIENT_NAME_LENGTH) {
        // NOTE: `8` is the ASCII code for Backspace and 127 is DEL
        int difference = !(c == 8 || c == 127) ? 1 : -1;
        client_name_length =
            CLAMP(client_name_length + difference, 1, MAX_CLIENT_NAME_LENGTH);
        if (difference == -1) {
          client_name_length = strlen(client->client_name);
          client->client_name[--client_name_length] = '\0';
        } else
          client->client_name[client_name_length - 1] = c;
      } else if (c == NEWLINE_KEY ||
                 client_name_length + 1 == MAX_CLIENT_NAME_LENGTH) {
        uint8_t trimmed_amount = trim_whitespace(client->client_name);

        // We can only carry on if the user entered something that
        // wasn't just whitespace

        if (trimmed_amount < client_name_length) {
          requires_username = FALSE;
          char *encoded_username = serialize_string(client->client_name);
          while (send(fds[0].fd, encoded_username, 1024, 0) < 1)
            ;
          free(encoded_username);

          // NOTE: We need to wait for a heads-up from the server that we were
          // successful.
          recv(fds[0].fd, buffer, 1024, 0);
          if (deserialize_int(buffer) == 1) {
            printf("\033[%d;0H", 6); // Move to the line above the input dialog
            printf("\033[s");
            printf("\033[J"); // Clear the screen below that line
            requires_username = FALSE;
          }
        }
        client_name_length -= trimmed_amount;
      }

      // NOTE: This may have been changed in an above statement
      // which is why we must check it once more.
      if (requires_username) {
        printf("\033[%d;0H", 8); // move cursor to y-coordinate 8

        // NOTE: We need to clear the whole line before re-drawing
        // to prevent excess chars that have been deleted from rendering
        printf("\033[2K");

        printf("%s\r\n", client->client_name);
        printf("\033[u"); // restore cursor position
      }

      fflush(stdout);

    } else if (input_status > 0) {
      switch (c) {
      case QUIT_KEY:
      case CTRL_C_KEY:
        fds[0].fd = -1; // NOTE: This will prevent polls from occuring and
                        // will break our loop
        break;
      }
    }
  }

  disable_raw_term();
  client_disconnect(client);
  return 0;
}

void hide_term_cursor() { printf("\033[?25l"); }
void show_term_cursor() { printf("\033[?25h"); }

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
  hide_term_cursor();
}

void disable_raw_term() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &restore);
  show_term_cursor();
}

client_t *client_init() {
  client_t *client = (client_t *)malloc(sizeof(client_t));
  int socket_fd = socket(AF_INET, SOCK_STREAM, TCP);
  if (socket_fd == -1) {
    handle_sock_error(errno);
    exit(1);
  }
  printf("\x1b[32;1mSocket created successfully\x1b[0m\n");
  int did_connect = client_connect(socket_fd, client) == 0 ? 1 : 0;
  if (!did_connect) {
    client->socket = socket_fd;
  }

  return client;
}

int client_connect(int server_fd, client_t *client) {
  printf("\x1b[33;1mAttempting to connect to server\x1b[0m\r\n");
  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_addr = INADDR_ANY,
                                    .sin_len = sizeof(server_addr),
                                    .sin_port = htons(PORT)};
  int connect_status =
      connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (connect_status == -1 && server_fd == client->socket) {
    // NOTE: This means that we have already attempted to connect to the
    // server and were not successful
    return -1;
  } else if (connect_status == -1) {
    handle_sock_error(errno);
    client_disconnect(client);
    exit(1);
  }

  client->socket = server_fd;
  client->addr = server_addr;

  // NOTE: THESE ARE DECIDED BY THE SERVER
  client->client_id = -1;
  client->client_name = calloc(MAX_CLIENT_NAME_LENGTH, sizeof(char));
  client->player_type = SPECTATOR;

  return 0;
}

void client_disconnect(client_t *client) {
  printf("\x1b[33;1mAttempting to disconnect from server\x1b[0m\n");
  int close_status = close(client->socket);
  if (close_status == -1) {
    handle_sock_error(errno);
    free(client->client_name);
    free(client);
    exit(1);
  }
  printf("\x1b[32;1mSuccessfully disconnected from server\x1b[0m\n");
  free(client->client_name);
  free(client);
}

void print_buffer(char *buf) {
  char *token = strtok(buf, "\n");
  while (token != NULL) {
    printf("%s\r\n", token);
    token = strtok(NULL, "\n");
  }
}
