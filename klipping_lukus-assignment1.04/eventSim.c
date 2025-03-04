#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/select.h>

#include "heap.h"
#include "dungeonGeneration.h"
#include "monster.h"
#include "character.h"
#include "path.h"
#include "eventSim.h"

void eventSim_update(dungeon_t *d)
{
    character_t *tmp;
    pair_t next_pos;
    int old_y = 0;
    int old_x = 0;
    monsters_generate(d);

    do
    {

        tmp = heap_remove_min(&d->heap);

        if (!tmp)
        {
            printf("Heap is empty! Ending simulation.\n");
            break;
        }
        // pc doesnt move yet
        if (tmp == d->PC)
        {
            tmp->sequence += (1000 / tmp->speed);
            heap_insert(&d->heap, tmp);
            continue;
        }
        old_y = tmp->position.y;
        old_x = tmp->position.x;

        d->character[tmp->position.y][tmp->position.x] = NULL;

        monster_next_position(d, tmp, &next_pos);

        if (d->PC && next_pos.x == d->PC->position.x && next_pos.y == d->PC->position.y)
        {
            d->PC->alive = 0;
            break;
        }

        // If monster is a not tunneler, leave corridors behind
        if (!(tmp->mon_character->characteristic & MON_TUNNEL))
        {
            d->map[old_y][old_x] = d->terrain[old_y][old_x];
        }
        else
        {
            if (d->terrain[old_y][old_x] != ROOM)
            {
                d->map[old_y][old_x] = HALL;
            }
            else
            {
                d->map[old_y][old_x] = d->terrain[old_y][old_x]; // restore original room
            }
        }

        if (d->character[next_pos.y][next_pos.x] != NULL)
        {
            next_pos.x = tmp->position.x;
            next_pos.y = tmp->position.y;
        }

        tmp->position.x = next_pos.x;
        tmp->position.y = next_pos.y;

        d->map[tmp->position.y][tmp->position.x] = tmp->symbol;

        tmp->sequence += (1000 / tmp->speed);

        heap_insert(&d->heap, tmp);
        if (tmp->position.y < 0 || tmp->position.y >= DUNGEON_Y ||
            tmp->position.x < 0 || tmp->position.x >= DUNGEON_X)
        {
            printf("ERROR: Attempted to access out-of-bounds position (%d, %d)\n",
                   tmp->position.y, tmp->position.x);
            return;
        }
        d->character[tmp->position.y][tmp->position.x] = tmp;

        dungeon_print(d);
        usleep(250000);
    } while (d->PC && d->PC->alive);

    printf("loss");
    heap_delete(&d->heap);

    delete_characterArray(d);
}
