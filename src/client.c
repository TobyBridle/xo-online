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
  int ignore_n_chars = 0;

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

        int received = smart_recv(client->socket, buffer, 1024);
        if (client_id == -1) {
          // We have received the client ID
          // We need to use strtol to convert the string to an integer
          /* client_id = deserialize_int(buffer); */
          client_id = strtol(buffer, NULL, 10);

          printf("\x1b[32;1mConnected to server as client %s\x1b[0m\r\n",
                 buffer);
          client->client_id = client_id;
          requires_username = TRUE;
          client->client_name = calloc(MAX_CLIENT_NAME_LENGTH, sizeof(char));
          print_buffer(clear_screen);
          print_buffer(main_menu);
        } else if (client->screen_state == IN_GAME_PAGE) {
          if (deserialize_int(buffer) == GAME_SIG_EXIT) {
            // We must leave the game.
            client->screen_state = GAME_VIEW_PAGE;
            print_buffer(game_end);
            sleep(1);
            print_buffer(clear_screen);

            // Send another request to the server
            // to retrieve the list of games.
            char *view_games_req = serialize_int(1);
            smart_send(fds[0].fd, view_games_req, 7);
            free(view_games_req);
          } else if (is_game_sig(buffer[0])) {
            // Switch out the buffer type so we can deserialize it
            int signal;
            switch (buffer[0]) {
            case GAME_SIG_CHECK:
            case GAME_SIG_CONFIRM:
            case GAME_SIG_EXIT:
              buffer[0] = INT_SERIALIZE_FLAG;
              signal = deserialize_int(
                  buffer); // Signal may be a position on the board
                           // or a signal to check for a condition (e.g win)
              break;
            default:
              signal = buffer[0];
            }
            handle_game_input(fds[0].fd, client, signal, ENEMY);
          }
        }
        if (client->client_name != NULL &&
            client->client_name[0] !=
                0) // The server doesn't send anything useful
                   // for output until after we have joined
        {
          print_buffer(buffer);
        }

        // Prevent previous long messages leaking into new short messages
        if (received > 0)
          bzero(buffer, received);
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
        printf("\0337");         // Save cursor position
        printf("\033[%d;0H", 7); // move cursor to y-coordinate 7
        printf("\033[;1mPlease Enter your Name:\x1b[;0m\r\n");
      }
      // We have had some input and after we have
      // finished our conditions we will have to flush the output.
      else if (c == CTRL_C_KEY) {
        fds[0].fd = -1;
        break;
      } else if (c != NEWLINE_KEY && is_valid_input_key(c) &&
                 client_name_length < MAX_CLIENT_NAME_LENGTH) {
        // NOTE: `8` is the ASCII code for Backspace and 127 is DEL
        int difference = !(c == 8 || c == 127) ? 1 : -1;
        client_name_length =
            CLAMP(client_name_length + difference, 1, MAX_CLIENT_NAME_LENGTH);
        if (difference == -1) {
          client_name_length =
              CLAMP(strlen(client->client_name), 1, MAX_CLIENT_NAME_LENGTH);
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
          serialized_string encoded_username =
              serialize_string(client->client_name);
          while (smart_send(fds[0].fd, encoded_username.str,
                            encoded_username.len + 1) < 1)
            ;
          free(encoded_username.str);

          // NOTE: We need to wait for a heads-up from the server that we were
          // successful.
          smart_recv(fds[0].fd, buffer, 1024);
          if (deserialize_int(buffer) == 1) {
            printf("\033[%d;0H", 6); // Move to the line above the input dialog
            printf("\0337");
            printf("\033[J"); // Clear the screen below that line
            client->screen_state = HOME_PAGE;
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
        printf("\0338"); // restore cursor position
      }

      fflush(stdout);

    } else if (input_status > 0) {
      if (ignore_n_chars > 0) {
        ignore_n_chars--;
        continue;
      }
      serialized s;
      switch (c) {
      case ESC_KEY:
        ignore_n_chars = 2;
        break;
      case QUIT_KEY:
      case CTRL_C_KEY:
        fds[0].fd = -1; // NOTE: This will prevent polls from occuring and
                        // will break our loop
        break;
      case 'r':
      case 'R':
        if (client->screen_state == GAME_VIEW_PAGE) {
          s.val = serialize_int(1);
          smart_send(fds[0].fd, s.val, 7);
          free(s.val);
          s.val = NULL;
        }
        break;
      case 'b':
      case 'B':
        if (client->screen_state == GAME_VIEW_PAGE) {
          smart_send(fds[0].fd, "b", 2);
          client->screen_state = HOME_PAGE;
          print_buffer(clear_screen);
          print_buffer(main_menu);
        } else if (client->screen_state == IN_GAME_PAGE) {
          print_buffer(clear_screen);
          smart_send(fds[0].fd, "b", 2);
          client->screen_state = GAME_VIEW_PAGE;
        }
        break;
      case ' ':
        if (client->screen_state == GAME_VIEW_PAGE) {
          s.str = serialize_string(" ");
          int sent;
          while ((sent = smart_send(fds[0].fd, s.str.str, s.str.len + 1)) !=
                 s.str.len + 1)
            ;
          free(s.str.str);

          // We might not be able to join the game.
          // If we have not refreshed, the server will send us back a response
          // with the updated games list.
          int received = smart_recv(fds[0].fd, buffer, 1024);
          if (received > 0) {
            s.str.str = deserialize_string(buffer);
            if (!strcmp(s.str.str, "joined")) {
              client->screen_state = IN_GAME_PAGE;
              game_t *game = calloc(1, sizeof(game_t));
              game->isCurrentPlayerTurn =
                  FALSE; // The host will be going first.
              client->game = game;
              setup_game_dep();
            }
            free(s.str.str);
          }
        }
        break;
      case '1':
        switch (client->screen_state) {
        case HOME_PAGE:
          view_active_games(fds[0].fd, client, &s);
          break;
        case IN_GAME_PAGE:
          handle_game_input(fds[0].fd, client, 1, PLAYER);
          break;
        default:
          break;
        }
        break;
      case '2':
        switch (client->screen_state) {
        case HOME_PAGE:
          create_new_game(fds[0].fd, client, &s);
          break;
        case IN_GAME_PAGE:
          handle_game_input(fds[0].fd, client, 2, PLAYER);
          break;
        default:
          break;
        }
        break;
      case '3':
        switch (client->screen_state) {
        case HOME_PAGE:
          fds[0].fd = -1;
          break;
        case IN_GAME_PAGE:
          handle_game_input(fds[0].fd, client, 3, PLAYER);
          break;
        default:
          break;
        }
        break;
      case '4':
        if (client->screen_state == IN_GAME_PAGE)
          handle_game_input(fds[0].fd, client, 4, PLAYER);
        break;
      case '5':
        if (client->screen_state == IN_GAME_PAGE)
          handle_game_input(fds[0].fd, client, 5, PLAYER);
        break;
      case '6':
        if (client->screen_state == IN_GAME_PAGE)
          handle_game_input(fds[0].fd, client, 6, PLAYER);
        break;
      case '7':
        if (client->screen_state == IN_GAME_PAGE)
          handle_game_input(fds[0].fd, client, 7, PLAYER);
        break;
      case '8':
        if (client->screen_state == IN_GAME_PAGE)
          handle_game_input(fds[0].fd, client, 8, PLAYER);
        break;
      case '9':
        if (client->screen_state == IN_GAME_PAGE)
          handle_game_input(fds[0].fd, client, 9, PLAYER);
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
  memset(client, 0, sizeof(client_t));
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

  client->client_name = NULL;
  client->screen_state = SETUP_PAGE;
  client->game = NULL;

  return client;
}

int client_connect(int server_fd, client_t *client) {
  printf("\x1b[33;1mAttempting to connect to server\x1b[0m\r\n");
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1) {
    fprintf(stderr, "Could not initialise address!\n");
    exit(EXIT_FAILURE);
  }
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

  // Free the game board strings
  int row, col;
  for (int i = 0; i < BOARD_WIDTH * BOARD_WIDTH; i++) {
    row = i / 3;
    col = i % 3;
    if (board[row][col].type != SERVER)
      free(board[row][col].print_string);
  }
  free(client->game);
  free(client);
}

void view_active_games(int socket, client_t *client, serialized *s) {
  s->val = serialize_int(1);
  smart_send(socket, s->val, 7);
  free(s->val);
  s->val = NULL;
  client->screen_state = GAME_VIEW_PAGE;
}

void create_new_game(int socket, client_t *client, serialized *s) {
  s->val = serialize_int(2);
  smart_send(socket, s->val, 7);
  free(s->val);
  s->val = NULL;

  print_buffer(clear_screen);
  print_buffer(waiting_room);

  setup_game_dep();

  client->screen_state = IN_GAME_PAGE;
  game_t *game = calloc(1, sizeof(game_t));
  game->isCurrentPlayerTurn = TRUE; // The game host gets to go first.
  client->game = game;
}

void setup_game_dep() {
  int row, col;
  for (int i = 0; i < BOARD_WIDTH * BOARD_WIDTH; i++) {
    row = i / 3;
    col = i % 3;
    board[row][col] =
        (board_piece){.piece = (1 + row + col) + '0',
                      .type = SERVER}; // NOTE: This only works where
                                       // BOARD_WIDTH * BOARD_WIDTH < 10
    snprintf(intermediate, intermediate_mem + 1, "\x1b[30;1m%c\x1b[0;0m",
             (i + 1) + '0');
    board[row][col].print_string = strdup(intermediate);
  }

  update_board(PLAYER);
}

void handle_game_input(int socket, client_t *client, unsigned int position,
                       Source source) {
  // Define up here so we can check in the handle_game_win
  int game_over;

  // Check if the signal is related to the game state (e.g ending /
  // drawing)
  if (source == ENEMY && position == GAME_SIG_WIN || position == GAME_SIG_DRAW)
    goto handle_game_win;

  // We are attempting to play when it is not our turn
  if (source == PLAYER && !((game_t *)client->game)->isCurrentPlayerTurn)
    return;

  unsigned int row, col;
  row = (position - 1) / 3;
  col = (position - 1) % 3;

  // The other player is attempting to play when it is our turn.
  int is_other_player_attempting =
      source == ENEMY && ((game_t *)client->game)->isCurrentPlayerTurn;
  // The position is invalid (we only end up here if none of the signals
  // match ones such as GAME_SIG_WIN so this must have the intention of
  // being a position)
  int is_position_invalid =
      position > (BOARD_WIDTH * BOARD_WIDTH) || position < 1;

  if (is_other_player_attempting || is_position_invalid) {
    char *confirm_message = serialize_bool(FALSE);
    confirm_message[0] = GAME_SIG_CONFIRM;
    smart_send(socket, confirm_message, 4);
    free(confirm_message);
    return;
  } else if (board[row][col].type != SERVER) { // Position is already occupied.
    char *confirm_message = serialize_bool(FALSE);
    confirm_message[0] = GAME_SIG_CONFIRM;
    smart_send(socket, confirm_message, 4);
    free(confirm_message);
    return;
  }

  // All checks have been passed, update the board
  if (source == ENEMY) {
    char *confirm_message = serialize_bool(TRUE);
    confirm_message[0] = GAME_SIG_CONFIRM;
    smart_send(socket, confirm_message, 4);
    free(confirm_message);
    goto change_board;
  }

  // Check if the other client thinks the move is valid.
  char *check_message = serialize_int(position);
  // The server will check this for whether it's a game signal or a normal int.
  check_message[0] = GAME_SIG_CHECK;
  smart_send(socket, check_message, 7);
  free(check_message);

  char buf[4];
  // Wait for a response from the server.
  smart_recv(socket, buf, 4);

  // The move was not valid.
  buf[0] = BOOL_SERIALIZE_FLAG; // Change from GAME_SIG_CONFIRM to bool so we
                                // can deserialize
  if (deserialize_bool(buf) == FALSE) {
    return;
  }

change_board:

  board[row][col].piece = position;
  board[row][col].type = source;
  snprintf(intermediate, intermediate_mem + 1, "\x1b[3%d;1m%c\x1b[0;0m",
           source == PLAYER ? 2 : 1,
           source == PLAYER ? 'x' : 'o'); // 2 -> 32 -> Green, 1 -> 31 -> Red
  board[row][col].print_string = strdup(intermediate);

  update_board(source);

  printf("\0338");
  printf("\0337");
  printf("\033[B");
  print_buffer(printable_board);

  ((game_t *)client->game)->isCurrentPlayerTurn ^= 1; // Switch turns.
  printf("\0338");

  // We don't NEED to check anything else if we are just accepting an
  // enemy position
  if (source == ENEMY)
    return;

  // Check if we have won.
  game_over = is_game_over(PLAYER);
  if (game_over > 0) {
    char *check_game_over = serialize_bool(TRUE);
    check_game_over[0] = game_over == 1 ? GAME_SIG_WIN : GAME_SIG_DRAW;
    smart_send(socket, check_game_over, 4);

    // Wait for the response.
    char res[4] = {0};
    while (smart_recv(socket, res, 4) < 0 && res[0] != GAME_SIG_CONFIRM_END)
      ;

    res[0] = BOOL_SERIALIZE_FLAG;
    if (deserialize_bool(res) == TRUE) {
      // The other player agrees with our decision.
      printf("You have %s the game!\r\n", game_over == 1 ? "won" : "drawn");
      // Delay the program for 1s, then recieve the server message.
      sleep(1);
    }
    free(check_game_over);
  }
  return;

handle_game_win:
  game_over = is_game_over(ENEMY);
  // Convert from the defined values to the enum values (e.g 12 is Draw -> 2 for
  // Draw)
  if (game_over == (position - 10)) {
    // Send back a confirmation message to the enemy.
    char *game_win_confirmation = serialize_bool(TRUE);
    game_win_confirmation[0] = GAME_SIG_CONFIRM_END;
    smart_send(socket, game_win_confirmation, 4);
    free(game_win_confirmation);

    printf("\033[AYou have %s the game!\r\n",
           game_over == 1 ? "lost" : "drawn");
    sleep(1);
  } else {
    char *game_win_denial = serialize_bool(FALSE);
    game_win_denial[0] = GAME_SIG_CONFIRM_END;
    smart_send(socket, game_win_denial, 4);
    free(game_win_denial);
  }
}

void update_board(int source) {
  snprintf(printable_board, printable_board_mem + 1, board_template,
           board[0][0].print_string, board[0][1].print_string,
           board[0][2].print_string, board[1][0].print_string,
           board[1][1].print_string, board[1][2].print_string,
           board[2][0].print_string, board[2][1].print_string,
           board[2][2].print_string, source != PLAYER ? "currently" : "not");
}

int is_game_over(Source source) {
  // Since the player enum value is 0 and there are no other
  // values in the enum < 0, we can do !(val) to see if it is
  // the player value.
  int win_horizontal =
      IS_ROW_WON(0, source) || IS_ROW_WON(1, source) || IS_ROW_WON(2, source);
  if (win_horizontal)
    return 1;

  int win_vertical =
      IS_COL_WON(0, source) || IS_COL_WON(1, source) || IS_COL_WON(2, source);
  if (win_vertical)
    return 1;

  int win_diagonal =
      (board[0][0].type == source && board[1][1].type == source &&
       board[2][2].type == source) ||
      (board[2][0].type == source && board[1][1].type == source &&
       board[0][2].type == source);
  if (win_diagonal)
    return 1;

  // Check if any spaces are left
  BOOL is_space_left = FALSE;
  int row, col;
  for (int pos = 0; pos < BOARD_WIDTH * BOARD_WIDTH; pos++) {
    row = pos / 3;
    col = pos % 3;
    if (board[row][col].type == SERVER) {
      is_space_left = TRUE;
      break;
    }
  }

  if (!is_space_left)
    return 2;
  return 0;
}

void print_buffer(char *buf) {
  char *dup = strdup(buf);
  char *token = strtok(dup, "\n");
  while (token != NULL) {
    printf("%s\r\n", token);
    token = strtok(NULL, "\n");
  }
  free(dup);
}
