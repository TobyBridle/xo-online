#include "lib/server.h"

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

  // NOTE: We need to create an array of max size MAX_CLIENTS
  clients_t clients;
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
  client.client_id = server->conns.count++;
  // We need to send the client a message
  // in which we request their name
  // clinet.client_name = <whatever-the-response-is>
  client.addr = client_addr;
  // Spectator by default so that they cannot interact with any games.
  client.player_type = SPECTATOR;

  return client;
}

void server_serve(server_t *server) {
  // NOTE: `loop` is a macro defined in `lib/server.h`
  loop {
    // Accept a connection
    client_t client = server_accept(server);
    // The ID of the client corresponds to the index of the client in the array
    server->conns.clients[client.client_id] = client;
  }
  pthread_exit(NULL);
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
  int pipefd[2];
  // Initialise the pipe
  if (pipe(pipefd) == -1) {
    fprintf(stderr, "Failed to create pipe\n");
    exit(1);
  }
  int readfd = pipefd[0];
  int writefd = pipefd[1];
  pid_t child = fork();
  char buf[1024];
  // When child is forked, 0 is the return value.
  // Otherwise, it is the PID of the child.
  if (child != 0) {
    // Wait for the child process to send IPC message for "start"
    printf("Waiting for child to send IPC message for \"start\"\n");
    while (read(readfd, buf, sizeof(buf)) == 0)
      ;
    if (strcmp(buf, "start") == 0) {
      // Erase the line that we just printed
      printf("\x1b[1A\033[K");
      handle_commands(server, child);
    }
  } else {
    pthread_create(&server_thread, NULL, (void *)server_serve, server);
    write(writefd, "start", 5);
  }
}

void handle_commands(server_t *server, pid_t subprocess) {
  printf("Commands:\n1. Terminate Server\n2. Print current games\n");
  loop {
    char cmd = getchar();
    switch (cmd) {
    case '1':
      printf("Terminating server...\n");
      kill(subprocess, SIGKILL);
      exit(0);
      break;
    case '2':
      printf("Current games:\n");
      break;
    default:
      printf("Invalid command\n");
      break;
    }
  }
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
  kill(getpid(), SIGKILL);
  return 0;
}
