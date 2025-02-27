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

int character_compare(const void *ch1, const void *ch2)
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

    int dx = abs(targetP.x - observeP.x);
    int dy = abs(targetP.y - observeP.y);

    // factor in sight length
    if (dx > SIGHT_DISTANCE || dy > SIGHT_DISTANCE)
    {
        // cannot see too far
        // monster needs glasses
        return 0;
    }

    // check differences
    if (targetP.y > observeP.y)
    {
        diff.y = targetP.y - observeP.y;
        dir.y = 1;
    }
    else
    {
        diff.y = observeP.y - targetP.y;
        dir.y = -1;
    }

    if (targetP.x > observeP.x)
    {
        diff.x = targetP.x - observeP.x;
        dir.x = 1;
    }
    else
    {
        diff.x = observeP.x - targetP.x;
        dir.x = -1;
    }

    if (diff.x > diff.y)
    {
        D = (2 * diff.y) - diff.x;
        for (i = 0; i <= diff.x; i++)
        {
            if (d->map[observeP.y][observeP.x] < ROOM && i && i != diff.x)
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
        for (i = 0; i <= diff.y; i++)
        {
            if (d->map[observeP.y][observeP.x] < ROOM && i && i != diff.y)
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
