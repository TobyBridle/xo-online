#include "lib/server.h"
#include "lib/client.h"
#include "lib/utils.h"

#define PORT 80

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
  printf("\x1b[33;1mAttempting to accept client\x1b[0m\n");
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_socket =
      accept(server->socket, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_socket == -1) {
    handle_sock_error(errno);
    exit(1);
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

  // We want to use `poll` to check for new connections
  struct pollfd fds[1];
  fds[0].fd = server->socket;
  fds[0].events = POLLIN;

  // If there is a new connection, we want to call
  // `server_accept` and add the client to the list of clients
  loop {
    // NOTE: First we need to poll the currently used file descriptors
    // This is because we want to get the next available id BEFORE there is a
    // connection rather than AFTER the connection. We can find the file
    // descriptors in the map (server->clients->entry_ids) to check if there
    // have been any disconnects After we have done this, we can begin
    // handling connections.

    BucketValue ret;
    client_t *client;
    while (entry_ids != NULL) {
      int client_id = entry_ids->data;
      ret = get(server->clients, client_id);
      if (ret.err == -1) {
        entry_ids = entry_ids->next;
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

      if (entry_ids != NULL)
        entry_ids = entry_ids->next;

    }

    if (poll(fds, 1, 100) > 0) {
      if (fds[0].revents & POLLIN) {
        client_t client = server_accept(server);
        printf("\x1b[32;1mClient %d connected successfully\x1b[0m\n",
               client.client_id);
        // Convert client id to string
        // id_size is the number of bytes used to store the number
        // in hex
        int length = snprintf(NULL, 0, "%d", client.client_id);
        char *client_id = calloc(length + 1, sizeof(char));
        sprintf(client_id, "%d", client.client_id);
        send(client.socket, client_id, length + 1, 0);
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
  fprintf(stderr,
          "\x1b[31;Cannot Unbind Server; Not Yet Implemented!\x1b[0m\n");
  return -1;
}
