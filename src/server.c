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

/* pthread_t server_thread; */
/* pthread_t game_handling_thread; */

int main() {
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
  server->state = ACCEPTING;
  printf("\x1b[33;1mAttempting to accept client\x1b[0m\n");
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_socket =
      accept(server->socket, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_socket == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  if (server->clients.used_buckets == MAX_CLIENTS) {
    send(client_socket, serialize_int(-1), 7, 0);
    server->state = NOT_ACCEPTING;
    return (client_t){.socket = -1, 0};
  }

  client->socket = client_socket;
  client->addr = client_addr;
  client->player_type = SPECTATOR;

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
      if ((uint)current->data != next_id) {
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
  signal(SIGINT, server_sigint);
  server->state = ACCEPTING;

  // We want to use `poll` to check for new connections
  struct pollfd fds[1];
  fds[0].fd = server->socket;
  fds[0].events = POLLIN;

  // If there is a new connection, we want to call
  // `server_accept` and add the client to the list of clients
  loop {

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
      int client_id = entry_id.data;
      ret = get(server->clients, client_id);
      if (ret.err == -1) {
        head = entry_id.next;
        continue;
      }
      client = ret.client;

      // Check if the client has disconnected
      char buf[1] = {0};
      int recv_status = recv(client->socket, buf, 1, MSG_PEEK);
      if (recv_status > 0) {
        // NOTE: Even though we do not register it,
        // we must consume the input so that it does not linger
        // and affect the recv calls if the client disconnects.
        recv(client->socket, NULL, 1024, 0);
      } else if (recv_status == 0) {
        // The client has disconnected
        printf("\x1b[31;1mClient %d has disconnected\x1b[0m\n", client_id);
        // Close the socket
        close(client->socket);
        // Free the client
        free(client);
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
        client_t client = server_accept(server);
        if (client.socket == -1) {
          continue;
        }
        printf("\x1b[32;1mClient %d connected successfully\x1b[0m\n",
               client.client_id);
        int length = snprintf(NULL, 0, "%d", client.client_id) + 1;
        char *client_id = calloc(length, sizeof(char));
        sprintf(client_id, "%d", client.client_id);
        send(client.socket, client_id, 1024, 0);

        send(client.socket, clear_screen.s_string, 1024, 0);
        send(client.socket, main_menu.s_string, 1024, 0);
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
  int entry_id;
  client_t *client;
  // NOTE: This block is almost identical
  // To the `free_hashmap(&map)` function, with the difference
  // being that we get the client from the map and then close it.
  while ((entry_id = pop_node(server->clients.entry_ids)) != -1) {
    client = get(server->clients, entry_id).client;
    close(client->socket);
    printf("\x1b[32;1mClosed connection from Client %d\x1b[0;0m\n",
           client->client_id);
    remove_value(&server->clients, entry_id);
  }

  free(server->clients.buckets);
  free(server->clients.entry_ids);
  free(server);

  printf("\x1b[33;1mAttempting to kill server instance now\x1b[0;0m\n");
  exit(0);
}
