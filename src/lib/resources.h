#ifndef NOUGHTS_CROSSES_RESOURES_H
#define NOUGHTS_CROSSES_RESOURES_H

#include <string.h>

#define StringResource static const char *

StringResource clear_screen = "\x1b[2J\x1b[H";
StringResource main_menu = "\x1b[;1mWelcome to XO Online!\n\n1)\tView Active "
                           "Games\n2)\tCreate new Game\n3)\tQuit\x1b[0;0m\n";

StringResource game_info_template = "%s's game\t[%d/2]\n";

StringResource view_games =
    "\x1b[32;1mThere %s currently %d "
    "available %s.\n\r\n\x1b[0;1m(SPACE) Join Game\n(R) Refresh "
    "Games\n(B) Go back to Home Page\n(Q) Quit\x1b[0;0m\n\r\n";

StringResource waiting_room =
    "\x1b[;1mYou're currently waiting for another player to join..\nB) Leave "
    "Room\nQ) Quit\x1b[0;0m\n\r\n\x1b[;3mIf you don't want to wait for too "
    "long, it might "
    "be best to leave the room\nand join someone someone else's "
    "game.\x1b[0;0m\n";

StringResource playing_header = "\x1b[;1mYou're playing against %s!\n";

const static char board_template[] = " %s | %s | %s\n"
                                     "---+---+---\n"
                                     " %s | %s | %s\n"
                                     "---+---+---\n"
                                     " %s | %s | %s\n";

StringResource prefilled =
    " \x1b[30;1m1\x1b[0;0m | \x1b[30;1m2\x1b[0;0m | \x1b[30;1m3\x1b[0;0m\n"
    "---+---+---\n"
    " \x1b[30;1m4\x1b[0;0m | \x1b[30;1m5\x1b[0;0m | \x1b[30;1m6\x1b[0;0m\n"
    "---+---+---\n"
    " \x1b[30;1m7\x1b[0;0m | \x1b[30;1m8\x1b[0;0m | \x1b[30;1m9\x1b[0;0m\n";

#endif
