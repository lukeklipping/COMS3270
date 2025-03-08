#include "move.h"

#include "dungeonGeneration.h"
#include "character.h"
#include "monster.h"
#include "pc.h"

/*TODO: Fix */
void move_turn_base(dungeon_t *d, uint32_t direction)
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
        /*
        if (tmp == d->PC)
        {
            tmp->sequence += (1000 / tmp->speed);
            heap_insert(&d->heap, tmp);
            continue;
        }*/
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

        tmp->position.x = next_pos.x;
        tmp->position.y = next_pos.y;

        d->map[tmp->position.y][tmp->position.x] = tmp->symbol;

        tmp->sequence += (1000 / tmp->speed);

        d->character[tmp->position.y][tmp->position.x] = tmp;

        heap_insert(&d->heap, tmp);

        dungeon_print(d);
        usleep(100000);
    } while (pc_alive(d));

    printf("\nyou lose, player dead\n\n");
}

/* Move character to new spot */
void move_character(dungeon_t *d, character_t *c, pair_t new)
{
    if (charpair(new) && ((new.y != c->position.y) ||
                          (new.x) != c->position.x))
    {
        // combat
    }
    else
    {
        // empty spot
        d->character[c->position.y][c->position.x] = NULL;
        c->position.y = new.y;
        c->position.x = new.x;
        d->character[c->position.y][c->position.x] = c;
    }
}

/* Move pc based on key presses */
int move_pc(dungeon_t *d, int direction)
{
    pair_t nextPos;
    int onStairs = 0;

    nextPos.x = d->PC->position.x;
    nextPos.y = d->PC->position.y;

    switch (direction)
    {
    case 7:
    case 'y':
        nextPos.y--;
        nextPos.x--;
        break;
    case 8:
    case 'k':
        nextPos.y--;
        break;
    case 9:
    case 'u':
        nextPos.y--;
        nextPos.x++;
        break;
    case 6:
    case 'l':
        nextPos.x++;
        break;
    case 3:
    case 'n':
        nextPos.y++;
        nextPos.x++;
        break;
    case 2:
    case 'j':
        nextPos.y++;
        break;
    case 1:
    case 'b':
        nextPos.y++;
        nextPos.x--;
        break;
    case 4:
    case 'h':
        nextPos.x--;
        break;
    case 5:
    case ' ':
    case '.':
        break;
    case '>':
        if (mappair(d->PC->position) == '<')
        {
            onStairs = 1;
            dungeon_new(d);
        }
        break;
    case '<':
        break;
        if (mappair(d->PC->position) == '<')
        {
            onStairs = 1;
            dungeon_new(d);
        }
    }

    if (onStairs)
    {
        return 0;
    }
    if ((direction != '>') && (direction != '<') &&
        ((mappair(nextPos) == ROOM) || (mappair(nextPos) == HALL)))
    {
        move_character(d, &d->PC, nextPos);
        return 0;
    }
    return 1;
}
