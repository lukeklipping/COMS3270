#include <ncurses.h>

#include "io.h"
#include "dungeon.h"
#include "dims.h"
#include "character.h"

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

void io_display_list(dungeon_t *d)
{
    int max_y, max_x, y, x;
    getmaxyx(stdscr, max_y, max_x); // Get terminal size

    // terminal vals
    int list_start_y = 1;
    int list_start_x = 1;
    int line = 0;

    int pc_y = d->pc.position[dim_y];
    int pc_x = d->pc.position[dim_x];

    int scroll_offset = 0;
    int key;

    do
    {
        for (y = 0; y < DUNGEON_Y && line < max_y; y++)
        {
            for (x = 0; x < DUNGEON_X && line < max_y; x++)
            {
                character_t *c = d->character[y][x];
                if (c == NULL || (y == pc_y && x == pc_x))
                {
                    continue;
                }

                int dis_x = pc_x - x;
                int dis_y = pc_y - y;
            }
        }

        refresh();

        // Handle input
        key = getch();
        if (key == KEY_UP && scroll_offset > 0)
        {
            scroll_offset--; // Scroll up if not at top
        }
        else if (key == KEY_DOWN && scroll_offset < d->num_monsters)
        {
            scroll_offset++; // Scroll down if not at bottom
        }

    } while (key != 27); // esc
}