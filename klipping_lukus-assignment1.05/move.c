#include "move.h"

#include "dungeonGeneration.h"
#include "character.h"
#include "monster.h"
#include "pc.h"
#include "eventSim.h"
#include "path.h"

#include <stdint.h>
#include <stdlib.h>

void do_combat(dungeon_t *d, character_t *atk, character_t *def)
{
    if (def->alive)
    {
        def->alive = 0;
        if (def != &d->PC)
        {
            d->num_monsters--;
        }
        atk->kills[kill_direct]++;
        atk->kills[kill_avenged] += (def->kills[kill_direct] +
                                     def->kills[kill_avenged]);
    }
}

void move_turn_base(dungeon_t *d) // uint32_t direction
{
    character_t *tmp;
    pair_t next_pos;
    event_t *e;

    /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
     * use to completely uninit the heap when generating a new level without *
     * worrying about deleting the PC.                                       */
    // int old_y = 0;
    // int old_x = 0;

    if (pc_alive(d))
    {
        e = malloc(sizeof(*e));
        e->type = event_character_turn;
        if (d->is_new)
        {
            d->is_new = 0;
            e->time = d->time;
        }
        else
        {
            e->time = d->time + (1000 / d->PC->speed);
        }
        e->sequence = 0;
        e->c = &d->PC;
        heap_insert(&d->heap, e);
    }

    while (pc_alive(d) &&
           (e = heap_remove_min(&d->heap)) &&
           ((e->type != event_character_turn) || (e->c != &d->PC)))
    {
        d->time = e->time;
        if (e->type == event_character_turn)
        {
            tmp = e->c;
        }
        if (!tmp->alive)
        {
            if (d->character[tmp->position.y][tmp->position.x] == tmp)
            {
                d->character[tmp->position.y][tmp->position.x] = NULL;
            }
            if (tmp != &d->PC)
            {
                event_delete(e);
            }
            continue;
        }

        monster_next_position(d, tmp, next_pos);
        move_character(d, tmp, next_pos);

        heap_insert(&d->heap, update_event(d, e, 1000 / tmp->speed));
    }

    if (pc_alive(d) && e->c == &d->PC)
    {
        tmp = e->c;
        d->time = e->time;
        /* Kind of kludgey, but because the PC is never in the queue when   *
         * we are outside of this function, the PC event has to get deleted *
         * and recreated every time we leave and re-enter this function.    */
        e->c = NULL;
        event_delete(e);
        pc_next_pos(d, next_pos);
        next_pos.x += tmp->position.x;
        next_pos.y += tmp->position.y;
        if (mappair(next_pos) == ROOM || mappair(next_pos) == ROCK)
        {
            mappair(next_pos) = HALL;
            d->hardness[next_pos.y][next_pos.x] = 0;
        }
        move_character(d, tmp, next_pos);

        djikstra_non_tunnel(d);
        djikstra_tunnel(d);
    }

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

/* From prof code */
void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir)
{
    dir.x = dir.y = 0;

    if (c->position.x != 1 && c->position.x != DUNGEON_X - 2)
    {
        dir.x = (c->position.x > DUNGEON_X - c->position.x ? 1 : -1);
    }
    if (c->position.y != 1 && c->position.y != DUNGEON_Y - 2)
    {
        dir.y = (c->position.y > DUNGEON_Y - c->position.y ? 1 : -1);
    }
}

/* From prof code */
uint32_t against_wall(dungeon_t *d, character_t *c)
{
    return ((mapxy(c->position.x - 1,
                   c->position.y) == IMMUTABLE_WALL) ||
            (mapxy(c->position.x + 1,
                   c->position.y) == IMMUTABLE_WALL) ||
            (mapxy(c->position.x,
                   c->position.y - 1) == IMMUTABLE_WALL) ||
            (mapxy(c->position.x,
                   c->position.y + 1) == IMMUTABLE_WALL));
}

/* From prof code */
uint32_t in_corner(dungeon_t *d, character_t *c)
{
    uint32_t num_immutable;

    num_immutable = 0;

    num_immutable += (mapxy(c->position.x - 1,
                            c->position.y) == IMMUTABLE_WALL);
    num_immutable += (mapxy(c->position.x + 1,
                            c->position.y) == IMMUTABLE_WALL);
    num_immutable += (mapxy(c->position.x,
                            c->position.y - 1) == IMMUTABLE_WALL);
    num_immutable += (mapxy(c->position.x,
                            c->position.y + 1) == IMMUTABLE_WALL);

    return num_immutable > 1;
}
