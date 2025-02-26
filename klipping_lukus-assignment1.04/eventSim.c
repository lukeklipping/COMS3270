
#include "dungeonGeneration.h"
#include "monster.h"

void eventSim_update(dungeon_t *d)
{
    heap_t heap;
    character_t *tmp;

    heap_init(&heap, monster_comp, NULL);
    heap_insert(&heap, &d->PC);
    monsters_generate(&d, &heap);

    int game = 1;

    while (game)
    {
        tmp = heap_remove_min(&heap);
        monster_next_pos(d, tmp, //)
    }
}