#ifndef NOUGHTS_CROSSES_RESOURES_H
#define NOUGHTS_CROSSES_RESOURES_H

#include <string.h>

typedef struct {
  size_t len;
  const char *s_string;
} StringResource;

const static StringResource clear_screen = {.len = 12,
                                            .s_string = "\x1b[2J\x1b[H"};

const static StringResource main_menu = {
    .len = 64,
    .s_string = "\x1b[;1mWelcome to XO Online!\n\n1)\tView Active "
                "Games\n2)\tCreate new Game\n3)\tQuit\x1b[0;0m\n"};

const static StringResource view_games = {
    .len = 64,
    .s_string = "Keys:\t(j) - Navigate Down\t(k) - Navigate Up\n\t(ENTER) - "
                "Join / Spectate Game\n\r\n\x1b[;1mThere %s currently %d "
                "available %s.\n(R) Refresh "
                "Games\n(B) Go back to Home Page\n(Q) Quit\x1b[0;0m\n\r\n"};

#endif
