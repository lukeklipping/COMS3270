#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "path.h"
#include "dungeonGeneration.h"
#include "character.h"
#include "readWriteDungeon.h"
#include "eventSim.h"

// generates dungeon for --rand
void dungeon_generate(dungeon_t *d)
{
    generate_rooms_character(d);
    generate_corridor(d);
    generate_stairs(d);
}

// usage flags for command line
void usage(const char *s)
{
    fprintf(stderr, "%s [--load|--save|--rand]", s);
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
        break;
    default:
        usage(argv[0]);
        return 1;
    }
    generate_PC(&dungeon);
    dungeon_print(&dungeon);

    // djikstra_tunnel(&dungeon);
    // djikstra_non_tunnel(&dungeon);
    //  printf("\nNon-Tunnel map\n");
    //  non_tunnel_map(&dungeon);
    //  printf("\nTunnel map\n");
    //  tunnel_map(&dungeon);
    eventSim_update(&dungeon);
    printf("Starting eventSim_update\n");
    printf("Starting eventSim_update\n");

    delete_dungeon(&dungeon);

    return 0;
}
