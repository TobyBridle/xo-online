#include "lib/server.h"
#include "lib/client.h"
#include "lib/utils.h"

#define PORT 80

volatile sig_atomic_t server_interrupted = 0;

pthread_t server_thread;
/* pthread_t game_handling_thread; */

void server_sigint(int sig) {
  printf("\x1b[33;1mAttempting to disconnect from server\x1b[0m\n");
  server_interrupted = 1;
}

int main(int argc, char *argv[]) {
  // Initialize the server
  server_t *server = server_init(PORT);
  int listen_status = server_listen(server);

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
  stck_t *available_ids = init_stack(MAX_CLIENTS);
  for (int i = MAX_CLIENTS; i > 0; i--) {
    push(available_ids, i);
  }
  server->clients = clients;
  server->available_ids = available_ids;

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
  printf("\x1b[33;1mAttempting to accept client\x1b[0m\n");
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_socket =
      accept(server->socket, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_socket == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  printf("\x1b[32;1mClient accepted successfully\x1b[0m\n");
  client_t *client = malloc(sizeof(client_t));
  client->socket = client_socket;
  client->addr = client_addr;
  client->player_type = SPECTATOR;
  client->client_id = pop(server->available_ids);

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
    struct node *current_id = server->clients.entry_ids->head;
    BucketValue ret;
    client_t *client = NULL;
    printf("\033c");
    while (current_id != NULL) {
      int client_id = current_id->data;
      ret = get(server->clients, client_id);
      if (ret.err != -1) {
        client = ret.client;
        printf("Client %d: %d\n", client_id, client->socket);

        // We can check if the client has disconnected by attempting
        // to read from the socket
        char buf[1];
        int read_status = recv(client->socket, buf, 1, MSG_PEEK);
        if (read_status == 0) {
          // The client has disconnected
          printf("\x1b[31;1mClient %d has disconnected\x1b[0m\n", client_id);
          // We want to close the socket
          close(client->socket);
          // We want to free the client
          free(client);
          // We want to remove the client from the map
          remove_value(server->clients, client_id);
          // We want to push the client id back onto the stack
          push(server->available_ids, client_id);
        }
      }
      current_id = current_id->next;
    }

    if (poll(fds, 1, 100) > 0) {
      if (fds[0].revents & POLLIN) {
        client_t client = server_accept(server);
        printf("\x1b[32;1mClient connected successfully\x1b[0m\n");
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
