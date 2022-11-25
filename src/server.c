#include "lib/server.h"
#include <arpa/inet.h>

const int MAX_CLIENTS = 1000;
#define PORT 80

pthread_t server_thread;
/* pthread_t game_handling_thread; */

int main(int argc, char *argv[]) {
  // Initialize the server
  server_t server = server_init(PORT);
  int listen_status = server_listen(&server);

  // Start the server and wait for connections
  server_start(&server);
  server_unbind(&server);
  return 0;
}

server_t server_init(short port) {
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
  // Set the socket opts
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  int bind_status =
      bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status == -1) {
    handle_sock_error(errno);
    exit(1);
  }
  printf("\x1b[32;1mSocket bound successfully\x1b[0m\n");

  // NOTE: We need to create an array of max size MAX_CLIENTS
  clients_t clients;
  clients.count = 0;
  server->conns = clients;

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
  struct sockaddr_in client_addr;
  socklen_t client_addr_size = sizeof(client_addr);
  int client_fd = accept(server->socket, (struct sockaddr *)&client_addr,
                         &client_addr_size);
  if (client_fd == -1) {
    handle_sock_error(errno);
    exit(1);
  }

  // If there is a connection, we need to check if it places us above our
  // threshold for maximum clients
  if (server->conns.count >= MAX_CLIENTS) {
    printf("\x1b[31;1mClient connection rejected; maximum clients "
           "reached\x1b[0m\n");
    return (client_t){0};
  }

  printf("\x1b[32;1mClient connected from %s:%d\x1b[0m\n",
         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  client_t client;
  client.socket = client_fd;
  client.client_id = server->conns.count++;
  // We need to send the client a message
  // in which we request their name
  // client.client_name = <whatever-the-response-is>
  client.addr = client_addr;
  // Spectator by default so that they cannot interact with any games.
  client.player_type = SPECTATOR;

  return client;
}

void server_serve(server_t *server) {
  printf("\x1b[33;1mAttempting to serve client\x1b[0m\n");

  char buf[1024];
  // NOTE: `loop` is a macro defined in `lib/server.h`
  loop {
    // Accept a connection
    client_t client = server_accept(server);
    server->conns.clients[client.client_id] = client;
  }
}

void server_start(server_t *server) {
  // Ignore SIGCHLD to avoid zombie threads
  signal(SIGCHLD, SIG_IGN);

  // Start a new thread for the server.
  // This thread will be responsible for accepting connections and
  // should run as a background process, passing the output to the main tty
  // using IPC. There should also be another thread for handling `n` games,
  // where `n` is pre-determined. The main thread should be used for handling
  // the servers state using commands.
  server_serve(server);
}

int server_unbind(server_t *server) {
  // We need to free the address and then
  // any memory used
  server->port = 0;
  int shutdown_status = shutdown(server->socket, SHUT_RDWR);
  if (shutdown_status == -1) {
    return shutdown_status;
  }
  server->socket = 0;
  free(server);
  kill(getpid(), SIGTERM);
  return 0;
}
