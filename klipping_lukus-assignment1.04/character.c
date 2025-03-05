#include <stdlib.h>

#include "character.h"
#include "dungeonGeneration.h"
#include "heap.h"
#include "monster.h"

void character_delete(void *c)
{
    character_t *del;
    if (c)
    {
        del = c;
        if (del->mon_character)
        {
            monsters_delete(del->mon_character);
        }
        free(del);
    }
}

int32_t character_compare(const void *ch1, const void *ch2)
{
    return ((const character_t *)ch1)->sequence - ((const character_t *)ch2)->sequence;
}

/*
Can see algorithm found online
Bresenhams line algo
https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
*/
int character_see(dungeon_t *d, character_t *observer, character_t *target)
{

    pair_t observeP, targetP, diff, dir;

    int D, i;
    // find coords
    targetP.x = target->position.x;
    targetP.y = target->position.y;
    observeP.y = observer->position.y;
    observeP.x = observer->position.x;

    // Boundary check: If out of dungeon bounds, return 0 (cannot see)
    if (targetP.x < 0 || targetP.x >= DUNGEON_X || targetP.y < 0 || targetP.y >= DUNGEON_Y ||
        observeP.x < 0 || observeP.x >= DUNGEON_X || observeP.y < 0 || observeP.y >= DUNGEON_Y)
    {
        return 0;
    }

    diff.x = abs(targetP.x - observeP.x);
    diff.y = abs(targetP.y - observeP.y);
    dir.x = (targetP.x > observeP.x) ? 1 : -1;
    dir.y = (targetP.y > observeP.y) ? 1 : -1;

    // factor in sight length
    if (diff.x > SIGHT_DISTANCE || diff.y > SIGHT_DISTANCE)
    {
        // cannot see too far
        // monster needs glasses
        return 0;
    }

    if (diff.x > diff.y)
    {
        D = (2 * diff.y) - diff.x;
        for (i = 1; i < diff.x; i++)
        {
            if (d->map[observeP.y][observeP.x] == ROCK || d->map[observeP.y][observeP.x] == IMMUTABLE_WALL) //&& i && i != diff.x
            {
                return 0;
            }
            observeP.x += dir.x;
            if (D < 0)
            {
                D += 2 * diff.y;
            }
            else
            {
                D += 2 * (diff.y - diff.x);
                observeP.y += dir.y;
            }
        }
    }
    else
    {
        D = (2 * diff.x) - diff.y;
        for (i = 1; i < diff.y; i++)
        {
            if (d->map[observeP.y][observeP.x] == ROCK || d->map[observeP.y][observeP.x] == IMMUTABLE_WALL) ///&& i && i != diff.y
            {
                return 0;
            }
            observeP.y += dir.y;
            if (D < 0)
            {
                D += 2 * diff.x;
            }
            else
            {
                D += 2 * (diff.x - diff.y);
                observeP.x += dir.x;
            }
        }
    }
    return 1;
}
/*
int same_room(dungeon_t *d, character_t *observer, character_t *target)
{
    for (int i = 0; i < d->num_rooms; i++)
    {
        Room *r = &d->rooms[i];

        if (observer->position.x >= r->x && observer->position.x < r->x + r->width &&
            observer->position.y >= r->y && observer->position.y < r->y + r->height &&
            target->position.x >= r->x && target->position.x < r->x + r->width &&
            target->position.y >= r->y && target->position.y < r->y + r->height)
        {
            return 1; // Same room
        }
    }
    return 0;
}*/