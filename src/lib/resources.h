#ifndef NOUGHTS_CROSSES_RESOURES_H
#define NOUGHTS_CROSSES_RESOURES_H

#include "utils.h"
#include <string.h>

typedef struct {
  size_t len;
  const char *s_string;
} StringResource;

const static StringResource clear_screen = {.len = 12, .s_string = "\x1b[2J\x1b[H"};

const static StringResource main_menu = {
    .len = 64,
    .s_string = "\x1b[;1mWelcome to XO Online!\n\n1)\tView Active Games\n2)\tQuit\x1b[0;0m\n"};
#endif
