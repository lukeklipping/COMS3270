#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "heap.h"
#include "dungeonGeneration.h"
#include "monster.h"
#include "character.h"

void eventSim_update(dungeon_t *d)
{
    heap_t heap;
    character_t *tmp;
    pair_t next_pos;
    memset(&next_pos, 0, sizeof(pair_t)); // This ensures all fields start at zero.
    int seq;

    heap_init(&heap, character_compare, NULL);
    heap_insert(&heap, d->PC);
    monsters_generate(d, &heap);

    while (heap.size)
    {
        tmp = heap_remove_min(&heap);
        seq = tmp->sequence;

        // pc died
        // will implement later
        if (!(d->PC->alive))
        {
            printf("dead lol\n");
            break;
        }
        if (monsters_number(d) == 0)
        {
            printf("win");
            break;
        }
        monster_next_position(d, tmp, &next_pos);

        tmp->sequence = seq + (1000 / tmp->speed);
        heap_insert(&heap, tmp);

        /* If pc moves, djikstras must be recalled  */

        /*
        if(tmp == d->PC){
            djikstra tunnel
            djikkstra non tunnel
        }
        */
        dungeon_print(d);
        usleep(250000);
    }
    heap_delete(&heap);
}