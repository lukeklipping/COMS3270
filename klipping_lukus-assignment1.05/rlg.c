#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>
// #include <sys/time.h>
#include <unistd.h>

#include "path.h"
#include "dungeonGeneration.h"
#include "character.h"
#include "readWriteDungeon.h"
#include "eventSim.h"
#include "pc.h"
#include "move.h"
#include "io.h"
#include "monster.h"

const char *victory =
    "\n                                       o\n"
    "                                      $\"\"$o\n"
    "                                     $\"  $$\n"
    "                                      $$$$\n"
    "                                      o \"$o\n"
    "                                     o\"  \"$\n"
    "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
    "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
    "\"oo   o o o o\n"
    "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
    "   \"o$$\"    o$$\n"
    "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
    "     o\"\"\n"
    "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
    "   o\"\n"
    "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
    "\"$$$  $\n"
    "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
    "\"\"      \"\"\" \"\n"
    "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
    "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
    "\"$$$$\n"
    "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
    "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
    "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
    "              $\"                                                 \"$\n"
    "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
    "$\"$\"$\"$\"$\"$\"$\"$\n"
    "                                   You win!\n\n";

const char *tombstone =
    "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
    "               /     /         \\             __\n"
    "              /     /           \\            ||\n"
    "             /____ /   Rest in   \\           ||\n"
    "            |     |    Pieces     |          ||\n"
    "            |     |               |          ||\n"
    "            |     |   A. Luser    |          ||\n"
    "            |     |               |          ||\n"
    "            |     |     * *   * * |         _||_\n"
    "            |     |     *\\/* *\\/* |        | TT |\n"
    "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
    "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
    "            |     |         \\/..\"\"\"\"\"...\"\"\""
    "\\ || /.\"\"\".......\"\"\"\"...\n"
    "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
    "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
    "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
    "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
    "            You're dead.  Better luck in the next life.\n\n\n";

// generates dungeon for --rand
void dungeon_generate(dungeon_t *d)
{
    generate_rooms(d);
    generate_corridor(d);
    generate_stairs(d);
}

// usage flags for command line
void usage(const char *s)
{
    fprintf(stderr, "%s [--load|--save|--rand|--nummon]", s);
}

// main
int main(int argc, char *argv[])
{
    int i;
    action_t action; // flags for command line
    char *saveFile = NULL, *loadFile = NULL;
    dungeon_t dungeon;
    srand(time(NULL));

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "--load") == 0)
            {
                if (i + 1 < argc)
                {
                    loadFile = argv[++i];
                    action = action_load;
                }
                else
                {
                    loadFile = NULL;
                    action = action_load;
                }
            }
            else if (strcmp(argv[i], "--save") == 0)
            {
                if (i + 1 < argc)
                {
                    saveFile = argv[++i];
                    action = action_save;
                }
                else
                {
                    saveFile = NULL;
                    action = action_save;
                }
            }
            else if (strcmp(argv[i], "--rand") == 0)
            {
                action = action_rand;
            }
            else if (strcmp(argv[i], "--nummon") == 0)
            {
                if (i + 1 < argc && sscanf(argv[i + 1], "%d", &dungeon.num_monsters) == 1)
                {
                    // will be auto rand
                    i++;
                    action = action_nummon;
                }
                else
                {
                    usage(argv[0]);
                }
            }
            else
            {
                usage(argv[0]); // unknown argument
                return 1;
            }
        }
    }
    else
    {
        usage(argv[0]);
        return 1;
    }

    dungeon_init(&dungeon);
    heap_init(&dungeon.heap, character_compare, NULL);

    switch (action)
    {
    case action_load:
        if (loadFile)
        {
            load_dungeon(&dungeon, loadFile);
        }
        else
        {
            load_dungeon(&dungeon, NULL);
        }
        break;
    case action_save:
        if (saveFile)
        {
            dungeon_generate(&dungeon);
            save_dungeon(&dungeon, saveFile);
        }
        else
        {
            dungeon_generate(&dungeon);
            save_dungeon(&dungeon, NULL);
        }
        break;
    case action_rand:
        dungeon_generate(&dungeon);
        dungeon.num_monsters = BASE_MONSTERS;
        break;
    case action_nummon:
        dungeon_generate(&dungeon);
        break;
    default:
        usage(argv[0]);
        return 1;
    }
    pc_place(&dungeon);
    pc_generate(&dungeon);
    monsters_generate(&dungeon);

    while (pc_alive(&dungeon) && monsters_number(&dungeon))
    {
        dungeon_print(&dungeon);
        move_turn_base(&dungeon);

        usleep(15000);
    }

    /*io_terminal_init();
    char key;

    do
    {
        // ncurses dungeon make
        key = getchar();
        move_turn_base(&dungeon, key);

    } while (pc_alive(&dungeon) && monsters_number(&dungeon));
    */

    printf("%s", pc_alive(&dungeon) ? victory : tombstone);
    printf("You defended your life in the face of %u deadly beasts.\n"
           "You avenged the cruel and untimely murders of %u "
           "peaceful dungeon residents.\n",
           dungeon.PC->kills[kill_direct], dungeon.PC->kills[kill_avenged]);

    pc_delete(dungeon.PC->pc);

    // delete_dungeon(&d);

    delete_dungeon(&dungeon);

    return 0;
}
