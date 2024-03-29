#ifndef NOUGHTS_CROSSES_CLIENT_H
#define NOUGHTS_CROSSES_CLIENT_H
#include "resources.h"
#include "utils.h"
#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define TCP 0
#define RECONNECT_INTERVAL                                                     \
  1 // This is the amount of seconds inbetween reconnect attempts

const uint8_t MAX_CLIENT_NAME_LENGTH = 24;

/* ------------------------------------------------------------------------ */

#ifndef QUIT_KEY
#define QUIT_KEY 'q'
#endif

#ifndef NEWLINE_KEY
#define NEWLINE_KEY 13
#endif

#ifndef CTRL_C_KEY
#define CTRL_C_KEY 3
#endif

#ifndef ESC_KEY
#define ESC_KEY 27
#endif

static struct termios restore;
static struct termios noughts_crosses_term;

void hide_term_cursor();
void show_term_cursor();

void enable_raw_term();
void disable_raw_term();

/* ------------------------------------------------------------------------ */

client_t *client_init();
int client_connect(int server_fd, client_t *client);
void client_disconnect(client_t *client);

#define BOARD_WIDTH 3

typedef enum { PLAYER = 0, ENEMY = 1, SERVER = 2 } Source;
typedef struct board_piece {
  char piece;
  Source type;        // In this instance, SERVER are uninitialised pieces.
  char *print_string; // The coloured string that is output.
} board_piece;

static board_piece board[BOARD_WIDTH][BOARD_WIDTH] = {0};

#define intermediate_mem (sizeof("\x1b[31;1m%s\x1b[0;0m"))
#define board_mem (BOARD_WIDTH * BOARD_WIDTH * intermediate_mem)
#define printable_board_mem (sizeof(board_template) + board_mem)

static char printable_board[printable_board_mem + 1]; // Each time we want
                                                      // to print, we will
// iterate over `board_template` and replace the %s with the correct string from
// the board.
static char intermediate[intermediate_mem + 1] = {0};

typedef struct {
  /* uint8_t game_id; */
  client_t *players[2];
  /* LinkedList spectators; */
  BOOL isCurrentPlayerTurn; // Either 0 or 1
  BOOL validConnections;
  BOOL isFull;
} game_t;

void view_active_games(int socket, client_t *client, serialized *s);
void create_new_game(int socket, client_t *client, serialized *s);
void setup_game_dep();
void handle_game_input(
    int socket, client_t *client, unsigned int position,
    Source source); // Position is 1-9, we then break this down into
                    // the 3x3 grid using MOD and DIV
void update_board(int source);

// Returns 0 if not, 1 if the game
// has ended in a win and 2 if the game was a draw.
int is_game_over(Source source);

#define IS_ROW_WON(row_num, source)                                            \
  (board[row_num][0].type == source && board[row_num][1].type == source &&     \
   board[row_num][2].type == source)

#define IS_COL_WON(col_num, source)                                            \
  (board[0][col_num].type == source && board[1][col_num].type == source &&     \
   board[2][col_num].type == source)

#ifndef HANDLE_SOCK_ERROR_FN
#define HANDLE_SOCK_ERROR_FN
/**
 * @brief Handle the errno provided by the socket failures
 *
 * @param err
 */
void handle_sock_error(int err);
#endif

void print_buffer(char *buf);
#endif
