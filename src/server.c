#include "lib/server.h"

#define PORT 80

int main(int argc, char *argv[]) {
  // Initialize the server
  server_t server = server_init(PORT);
  int listen_status = server_listen(&server);

  // Start the server and wait for connections
  server_serve(&server);
  server_unbind(&server);
  return 0;
}

server_t server_init(short port) {
  const int MAX_CLIENTS = 1000;
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
         server_addr.sin_port);
  // Set the socket opts
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  int bind_status =
      bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
    handle_sock_error(errno);
    exit(1);
  }
  printf("\x1b[32;1mSocket bound successfully\033[0m\n");

  // TODO: Create a hash table to store the currently connected clients
  return *server;
}

int server_listen(server_t *server) {
  // We need to listen on the socket
  printf("\x1b[33;1mAttempting to listen on socket\x1b[0m\n");
  int listen_status = listen(server->socket, 1000);
  if (listen_status == -1) {
    handle_sock_error(errno);
    exit(1);
  }
  printf("\x1b[32;1mSuccess; socket listening on port %d\x1b[0m\n", PORT);
  return listen_status;
}

client_t server_accept(server_t *server) {
  // Accept a connection
  printf("\x1b[33;1mWaiting to accept a connection...\x1b[0m\n");
  struct sockaddr_in client_addr;
  socklen_t client_addr_size = sizeof(client_addr);
  int client_fd = accept(server->socket, (struct sockaddr *)&client_addr,
                         &client_addr_size);
  if (client_fd == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  printf("\x1b[32;1mConnection accepted\x1b[0m\n");
  client_t client;
  client.socket = client_fd;
  client.addr = client_addr;
  return client;
}

void server_serve(server_t *server) {
  // NOTE: `loop` is a macro defined in `lib/server.h`
  loop {
    // Accept a connection
    client_t client = server_accept(server);
    /* TODO: Hash the client's name and place into server.clients
     * Maybe we can implement a utils.h file with tools such as creating
     * hashmaps and using them.
     * */
  }
}

int server_unbind(server_t *server) {
  // We need to free the address and then any memory used
  server->port = 0;
  int shutdown_status = shutdown(server->socket, SHUT_RDWR);
  if (shutdown_status == -1) {
    return shutdown_status;
  }
  server->socket = 0;
  free(server);
  return 0;
}
