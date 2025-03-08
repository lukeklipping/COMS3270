#include <ncurses.h>

#include "io.h"

void io_terminal_init(void)
{
    initscr();            // initializes terminal screen
    raw();                // passes input without 'enter'
    noecho();             // doesnt echo typed characters
    curs_set(0);          // shuts off mouse
    keypad(stdscr, TRUE); // keypad processing
    start_color();        // start color declaring below
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}