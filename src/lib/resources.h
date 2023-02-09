#ifndef NOUGHTS_CROSSES_RESOURES_H
#define NOUGHTS_CROSSES_RESOURES_H

#include <string.h>

#define StringResource const static char *

StringResource clear_screen = "\x1b[2J\x1b[H";
StringResource main_menu = "\x1b[;1mWelcome to XO Online!\n\n1)\tView Active "
                           "Games\n2)\tCreate new Game\n3)\tQuit\x1b[0;0m\n";

StringResource view_games =
    "Keys:\t(j) - Navigate Down\t(k) - Navigate Up\n\t(ENTER) - "
    "Join / Spectate Game\n\r\n\x1b[;1mThere %s currently %d "
    "available %s.\n(R) Refresh "
    "Games\n(B) Go back to Home Page\n(Q) Quit\x1b[0;0m\n\r\n";
#endif
