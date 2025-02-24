
#include "dungeonGeneration.h"
#include "monster.h"

void eventSim_update(dungeon_t *d)
{
    heap_t heap;
    character_t *tmp;

    heap_init(&heap, monster_comp, NULL);
    monsters_generate(&d, &heap);

    do
    {
        tmp = heap_remove_min(&heap);
        if (tmp->)
    } while ();
}