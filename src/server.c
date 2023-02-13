#include "lib/server.h"
#include "lib/client.h"
#include "lib/resources.h"
#include "lib/utils.h"

#define PORT 80

volatile sig_atomic_t server_interrupted = 0;

void server_sigint(int sig) {
  printf("\x1b[33;1mAttempting to disconnect from server\x1b[0m\n");
  server_interrupted = 1;
}

char *accepted_name_s_string, *rejected_name_s_string;

/* pthread_t server_thread; */
/* pthread_t game_handling_thread; */

int main() {

  accepted_name_s_string = serialize_int(1);
  rejected_name_s_string = serialize_int(-1);

  // Initialize the server
  server_t *server = server_init(PORT);
  int listen_status = server_listen(server);
  if (listen_status != 0) {
    handle_sock_error(errno);
    exit(1);
  }

  // Start the server and wait for connections
  server_start(server);
  server_unbind(server);
  return 0;
}

server_t *server_init(short port) {
  printf("\x1b[33;1mAttempting to initialise server on port %d\x1b[0m\n", port);
  server_t *server;
  server = (server_t *)malloc(sizeof(server_t));
  server->port = port;
  // We want to create a socket for the server
  int socket_fd = socket(AF_INET, SOCK_STREAM, TCP);
  if (socket_fd == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  // NOTE: We need to create a sockaddr_in struct to hold the address of the
  // server
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  server->socket = socket_fd;
  printf("\x1b[32;1mSocket created successfully\x1b[0m\n");

  // Bind the socket to the address
  printf("\x1b[33;1mAttempting to bind socket to address %hu\x1b[0m\n",
         htons(server_addr.sin_port));

  // Set socket to non-blocking
#ifdef _WIN32
  unsigned long mode = 1;
  ioctlsocket(socket_fd, FIONBIO, &mode);
#else
  int flags = fcntl(socket_fd, F_GETFL, 0);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
#endif

  // Set the socket to reuse the address
  int optval = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  int bind_status =
      bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  printf("\x1b[32;1mSocket bound successfully\x1b[0m\n");

  printf("\x1b[33;1mAttempting to create %d spaces for clients.\x1b[0m\n",
         MAX_CLIENTS);
  HashMap clients = new_hashmap(MAX_CLIENTS);
  server->clients = clients;
  server->games = init_list();

  printf("\x1b[32;1mClient spaces created successfully\x1b[0m\n");

  return server;
}

int server_listen(server_t *server) {
  printf("\x1b[33;1mAttempting to listen on socket\x1b[0m\n");
  int listen_status =
      listen(server->socket,
             32); // NOTE: The second param is the maximum backlog amount
  if (listen_status == -1) {
    handle_sock_error(errno);
    exit(1);
  }
  printf("\x1b[32;1mSocket listening successfully\x1b[0m\n");
  return listen_status;
}

client_t server_accept(server_t *server) {
  client_t *client = malloc(sizeof(client_t));
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int client_socket =
      accept(server->socket, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_socket == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  printf("\x1b[33;1mAttempting to accept client\x1b[0m\n");
  client->socket = client_socket;
  client->addr = client_addr;
  client->client_name = NULL;
  client->player_type = SPECTATOR;
  client->game = NULL;
  client->last_sent_game_hash = 0;
  client->screen_state = SETUP_PAGE;

  // We need to get the next available ID
  // Since the entry_ids are ordered in ascending
  // we can just loop over them and find the first
  // available ID
  struct node *current = server->clients.entry_ids->head;
  uint next_id = 1;

  // If there are no available IDs in gaps,
  // we can use the tail + 1
  if (current == NULL) {
    next_id = 1;
  } else {
    while (current != NULL) {
      if ((uint)current->data.i_value != next_id) {
        break;
      }
      next_id++;
      current = current->next;
    }
  }

  client->client_id = next_id;

  printf("\x1b[32;1mClient %d accepted successfully\x1b[0m\n",
         client->client_id);

  // We need to handle client/user creation
  // The ID of the client is assigned by popping the next
  // available ID from the stack
  put(&server->clients, client->client_id, (BucketValue){.client = client});
  return *client;
}

void server_serve(server_t *server) {
  printf("\x1b[33;1mAttempting to serve clients\x1b[0m\n");
  server->state = ACCEPTING;

  // We want to use `poll` to check for new connections
  struct pollfd fds[1];
  fds[0].fd = server->socket;
  fds[0].events = POLLIN;

  char peek_buf[1] = {0};
  char buf[1024] = {};

  // If there is a new connection, we want to call
  // `server_accept` and add the client to the list of clients
  loop {
    signal(SIGINT, server_sigint);
    if (server_interrupted) {
      server_unbind(server);
    }

    // NOTE: First we need to poll the currently used file descriptors
    // This is because we want to get the next available id BEFORE there is a
    // connection rather than AFTER the connection. We can find the file
    // descriptors in the map (server->clients->entry_ids) to check if there
    // have been any disconnects After we have done this, we can begin
    // handling connections.

    struct node entry_id;
    struct node *head = server->clients.entry_ids->head;
    // Pointer to next entry id
    struct node **next_entry_id = NULL;
    BucketValue ret;
    client_t *client;
    while (head != NULL) {
      entry_id = *head;
      if (entry_id.next == NULL) {
        next_entry_id = NULL;
      } else {
        next_entry_id = &entry_id.next;
      }
      int client_id = entry_id.data.i_value;
      ret = get(server->clients, client_id);
      if (ret.err == -1) {
        head = entry_id.next;
        continue;
      }
      client = ret.client;

      // Check if the client has disconnected
      int recv_status = recv(client->socket, peek_buf, 1, MSG_PEEK);
      if (recv_status > 0) {
        int received =
            smart_recv(client->socket, buf, 1024); // Accept the whole buffer

        // If the client does not have a name, we can assume that the buffer
        // contains their name.
        deserialized d;
        if (client->client_name == NULL) {
          char *name = deserialize_string(buf);
          uint8_t name_length = strlen(name);
          uint8_t trimmed_length = trim_whitespace(name);

          if (trimmed_length == name_length ||
              name_length > MAX_CLIENT_NAME_LENGTH) {
            smart_send(client->socket, rejected_name_s_string,
                       strlen(rejected_name_s_string) + 1);
          } else {
            client->client_name = name;
            smart_send(client->socket, accepted_name_s_string, 7);
            printf("Say hello to %s!\n", client->client_name);
            client->screen_state = HOME_PAGE;
          }
        } else if (deserialize_int(buf) == 1 &&
                   (client->screen_state == HOME_PAGE ||
                    client->screen_state == GAME_VIEW_PAGE)) {

          if (client->last_sent_game_hash !=
                  0 && // If it is not 0, the user has already had it rendered
                       // at some point
              client->last_sent_game_hash == server->current_game_hash) {
            continue;
          }

          struct node *head = server->games->head;
          game_t *game;

          int formatted_length = 0;
          char *formatted = NULL;

          unsigned long game_count = 0;
          LinkedList *games_string = init_list();
          while (head != NULL) {
            serialized *s = malloc(sizeof(serialized));

            game = head->data.pointer;
            if (game == NULL) {
              NEXT_ITER(head);
              continue;
            } else if (!game->validConnections) {
              // We need to shutdown the game

              // We need to keep track of the next one
              // as the current will be freed and unavailable to read from.
              NEXT_ITER(head);
              continue;
            }
            if (game->isFull) {
              NEXT_ITER(head);
              continue;
            }
            formatted_length =
                snprintf(NULL, 0, "%s's Game\t[%d/2]\n",
                         game->players[0]->client_name, game->isFull ? 2 : 1) +
                1;
            formatted = calloc(formatted_length, sizeof(char));
            sprintf(formatted, "%s's Game\t[%d/2]\n",
                    game->players[0]->client_name, game->isFull ? 2 : 1);
            s->str = serialize_string(formatted);

            push_node(games_string, (NodeValue){.pointer = &s->str});
            game_count++;
            free(formatted);
            formatted_length = 0;

            NEXT_ITER(head);
          }

          smart_send(client->socket, clear_screen, strlen(clear_screen) + 1);
          size_t header_length = snprintf(NULL, 0, view_games, HEADER_VERB,
                                          game_count, HEADER_GAME) +
                                 1;
          char *header = calloc(header_length, sizeof(char));
          sprintf(header, view_games, HEADER_VERB, game_count, HEADER_GAME);
          smart_send(client->socket, header, header_length);
          free(header);

          NodeValue val;
          while ((val = pop_node(games_string)).err != -1) {
            serialized_string *str = val.pointer;
            smart_send(client->socket, str->str, str->len);
            free(str->str);
            free(str);
          }
          free_list(games_string);
          client->last_sent_game_hash = server->current_game_hash;
        } else if (!strcmp(buf, "b") &&
                   client->screen_state == GAME_VIEW_PAGE) {
          client->screen_state = HOME_PAGE;
          client->last_sent_game_hash = 0;
        } else if (deserialize_int(buf) == 2 &&
                   client->screen_state == HOME_PAGE) {
          // Create New Game
          if (client->screen_state == IN_GAME_PAGE)
            return;
          game_t *game = malloc(sizeof(game_t));

          game->isFull = game->playerTurn = 0;
          /* game->spectators = *init_list(); */
          game->players[0] = client;
          game->validConnections = TRUE;

          push_node(server->games, (NodeValue){.pointer = game});
          server->current_game_hash = hash_games_list(server->games);
          client->game = game;
          client->screen_state = IN_GAME_PAGE;
        }
        if (received > 0)
          bzero(buf, received);
      } else if (recv_status == 0) {
        // The client has disconnected
        printf("\x1b[31;1mClient %d (%s) has disconnected\x1b[0m\n", client_id,
               client->client_name);
        // Close the socket
        while (recv(client->socket, NULL, 1024, 0) > 0)
          ;
        close(client->socket);

        if (client->client_name != NULL)
          free(client->client_name);
        // Free the client
        client->client_name = NULL;
        client->client_id = 0;
        if (client->game != NULL) {
          // We need to shut down the game if they're playing in one
          game_t *game = client->game;
          game->validConnections = FALSE;
          struct node *next_head = head->next;
          remove_node(server->games, (NodeValue){.pointer = game});

          server->current_game_hash =
              1; // Reset the hash to force a rehash of the games
          free(client->game);
          client->game = NULL;
        }
        free(client);
        client = NULL;
        // Remove the client from the hashmap
        remove_value(&server->clients, client_id);
      }

      // Set memory location of entry_id to the next entry id
      if (next_entry_id == &entry_id.next)
        head = entry_id.next;
      else
        head = NULL;
    }

    if (poll(fds, 1, 100) > 0) {
      if (fds[0].revents & POLLIN) {
        if (server->clients.used_buckets == MAX_CLIENTS) {
          continue;
        } else {
          server->state = ACCEPTING;
        }

        client_t client = server_accept(server);
        printf("\x1b[32;1mClient %d connected successfully\x1b[0m\n",
               client.client_id);
        int length = snprintf(NULL, 0, "%d", client.client_id) + 1;
        char *client_id = calloc(length, sizeof(char));
        sprintf(client_id, "%d", client.client_id);
        smart_send(client.socket, client_id, length);
        free(client_id);

        smart_send(client.socket, clear_screen, strlen(clear_screen) + 1);
        smart_send(client.socket, main_menu, strlen(main_menu) + 1);
      } else if (fds[0].revents & POLLERR) {
        printf("\x1b[31;1mError occurred\x1b[0m\n");
      }
    }
  }
}

void server_start(server_t *server) {
  printf("\x1b[33;1mAttempting to start server\x1b[0m\n");
  // We want to create a thread to handle the server
  server_serve(server);
}

int server_unbind(server_t *server) {
  // TODO: Implement freeing and closing of sockets.
  NodeValue entry_id;
  client_t *client;

  // NOTE: This block is almost identical
  // To the `free_hashmap(&map)` function, with the difference
  // being that we get the client from the map and then close it.
  while ((entry_id = pop_node(server->clients.entry_ids)).err != -1) {
    client = get(server->clients, entry_id.i_value).client;
    while (recv(client->socket, NULL, 1024, 0) > 0)
      ;
    close(client->socket);
    printf("\x1b[32;1mClosed connection from Client %d\x1b[0;0m\n",
           client->client_id);
    remove_value(&server->clients, entry_id.i_value);
  }

  free_hashmap(&server->clients);
  if (server->games != NULL)
    free_list(server->games);
  free(server);

  printf("\x1b[33;1mAttempting to kill server instance now\x1b[0;0m\n");
  exit(0);
}

unsigned long hash_games_list(LinkedList *games) {
  unsigned long hash = 0;
  unsigned int index = 0;

  struct node *curr = games->head;
  game_t *game;
  while (curr != NULL) {
    game = curr->data.pointer;
    hash = (hash * 31) + index * hash_string(game->players[0]->client_name, 67);
    hash += game->isFull ? index : 0;
    index++;
    NEXT_ITER(curr);
  }
  return hash + 17; // Just to ensure that it is never 0
}

int render_games_page(server_t *server, client_t *client) {

  if (client->last_sent_game_hash !=
          0 && // If it is not 0, the user has already had it rendered
               // at some point
      client->last_sent_game_hash == server->current_game_hash) {
    return -3;
  }

  struct node *head = server->games->head;
  game_t *game;

  int formatted_length = 0;
  char *formatted = NULL;

  unsigned long game_count = 0;
  LinkedList *games_string = init_list();
  while (head != NULL) {
    serialized *s = malloc(sizeof(serialized));

    game = head->data.pointer;
    if (game == NULL) {
      NEXT_ITER(head);
      continue;
    } else if (!game->validConnections) {
      // We need to shutdown the game

      // We need to keep track of the next one
      // as the current will be freed and unavailable to read from.
      struct node *next_head = head->next;
      remove_node(server->games, (NodeValue){.pointer = game});
      server->current_game_hash = hash_games_list(server->games);
      free(game);
      game = NULL;

      head = next_head;
      continue;
    }
    if (game->isFull) {
      NEXT_ITER(head);
      continue;
    }
    formatted_length =
        snprintf(NULL, 0, game_info_template, game->players[0]->client_name,
                 game->isFull ? 2 : 1) +
        1;
    formatted = calloc(formatted_length, sizeof(char));
    sprintf(formatted, game_info_template, game->players[0]->client_name,
            game->isFull ? 2 : 1);
    s->str = serialize_string(formatted);

    push_node(games_string, (NodeValue){.pointer = &s->str});
    game_count++;
    free(formatted);
    formatted_length = 0;

    NEXT_ITER(head);
  }

  smart_send(client->socket, clear_screen, strlen(clear_screen) + 1);
  size_t header_length =
      snprintf(NULL, 0, view_games, HEADER_VERB, game_count, HEADER_GAME) + 1;
  char *header = calloc(header_length, sizeof(char));
  sprintf(header, view_games, HEADER_VERB, game_count, HEADER_GAME);
  smart_send(client->socket, header, header_length);
  free(header);

  NodeValue val;
  while ((val = pop_node(games_string)).err != -1) {
    serialized_string *str = val.pointer;
    smart_send(client->socket, str->str, str->len);
    free(str->str);
    free(str);
  }
  free_list(games_string);
  client->last_sent_game_hash = server->current_game_hash;

  return 0;
}
