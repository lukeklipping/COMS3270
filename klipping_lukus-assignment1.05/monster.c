#include "path.h"
#include "heap.h"
#include "dungeonGeneration.h"
#include "monster.h"
#include "character.h"
#include "eventSim.h"

#include <stdlib.h>
#include <string.h>

void monsters_delete(monster_t *m)
{
    if (m)
    {
        free(m);
    }
}

static int is_not_occupied_by_monster(dungeon_t *d, int y, int x)
{
    // Ensure (y, x) is within dungeon bounds
    if (y < 0 || y >= DUNGEON_Y || x < 0 || x >= DUNGEON_X)
    {
        return 0;
    }

    if (d->map[y][x] == IMMUTABLE_WALL)
    {
        return 0;
    }

    // Check if there is a character at (y, x) and it's not the PC
    return d->character[y][x] && d->character[y][x] != d->PC;
}

static int is_walkable(dungeon_t *d, int x, int y)
{
    if ((mapxy(x, y) == ROOM ||
         mapxy(x, y) == HALL ||
         mapxy(x, y) == '<' ||
         mapxy(x, y) == '>'))
    {
        return 1;
    }
    return 0;
}

void monsters_generate(dungeon_t *d)
{
    int i;
    character_t *monster;
    int room;
    pair_t pos;
    const static char symbol[] = "0123456789ABCDEF";

    // d->num_monsters = 10;
    for (i = 0; i < d->num_monsters; i++)
    {
        monster = malloc(sizeof(*monster));
        memset(monster, 0, sizeof(*monster));

        // find location for pos
        do
        {
            room = RANDOM_RANGE(0, d->num_rooms - 1);
            pos.y = RANDOM_RANGE(d->rooms[room].y, d->rooms[room].y + d->rooms[room].height - 1);
            pos.x = RANDOM_RANGE(d->rooms[room].x, d->rooms[room].x + d->rooms[room].width - 1);
        } while (d->character[pos.y][pos.x]);
        // set characteristics
        monster->position = pos;

        d->character[pos.y][pos.x] = monster;

        monster->speed = RANDOM_RANGE(MONSTER_MAX_SPEED, MONSTER_MIN_SPEED);
        monster->alive = 1;
        monster->sequence = ++d->seq;
        monster->pc = NULL;

        monster->mon_character = malloc(sizeof(*monster->mon_character));

        monster->mon_character->characteristic = rand() & 0x0000000f;
        monster->symbol = symbol[monster->mon_character->characteristic];
        monster->mon_character->seen_PC = 0;
        monster->kills[kill_direct] = monster->kills[kill_avenged] = 0;

        d->character[pos.y][pos.x] = monster;

        heap_insert(&d->heap, new_event(d, event_character_turn, monster, 0));
    }
}

void monster_next_random(dungeon_t *d, character_t *c, pair_t next)
{
    pair_t dir;
    union
    {
        uint32_t i;
        uint8_t a[4];
    } r;
    do
    {
        dir.y = next.y;
        dir.x = next.x;
        r.i = rand();
        if (r.a[0] > 85 /* 255 / 3 */)
        {
            if (r.a[0] & 1)
            {
                dir.y--;
            }
            else
            {
                dir.y++;
            }
        }
        if (r.a[1] > 85 /* 255 / 3 */)
        {
            if (r.a[1] & 1)
            {
                dir.x--;
            }
            else
            {
                dir.x++;
            }
        }

    } while (mappair(dir) == IMMUTABLE_WALL || mappair(dir) == ROCK);

    next.y = dir.y;
    next.x = dir.x;
}

void monster_next_LOS(dungeon_t *d, character_t *c, pair_t next)
{
    pair_t dir;

    dir.y = d->PC->position.y - c->position.y;
    dir.x = d->PC->position.x - c->position.x;

    // normalize direction
    if (dir.y)
    {
        dir.y /= abs(dir.y);
    }
    if (dir.x)
    {
        dir.x /= abs(dir.x);
    }
    next.y = c->position.y;
    next.x = c->position.x;

    int new_y = next.y + dir.y;
    int new_x = next.x + dir.x;

    if (is_walkable(d, new_x, new_y))
    {
        // Move diagonally if possible
        next.y += dir.y;
        next.x += dir.x;
    }
    else if (is_walkable(d, new_x, next.y))
    {
        // Move horizontally
        next.x += dir.x;
    }
    else if (is_walkable(d, next.x, new_y))
    {
        // Move vertically
        next.y += dir.y;
    }
}
// uses djikstra again
void monster_next_rand_tunnel(dungeon_t *d, character_t *c, pair_t next)
{
    pair_t dir;
    do
    {
        dir.y = c->position.y + RANDOM_RANGE(-1, 1);
        dir.x = c->position.x + RANDOM_RANGE(-1, 1);
    } while ((d->map[dir.y][dir.x] == IMMUTABLE_WALL || is_not_occupied_by_monster(d, dir.y, dir.x)));

    // tunneller digs at wall by -85 each turn
    if (cellHardness(dir.y, dir.x) <= 85 && cellHardness(dir.y, dir.x) != 255)
    {
        d->hardness[dir.y][dir.x] = 0;
        if (d->map[c->position.y][c->position.x] != ROOM)
        {
            d->map[c->position.y][c->position.x] = HALL;
            // update maps
            djikstra_tunnel(d);
            djikstra_non_tunnel(d);
        }
        /*else if (d->map[c->position.y][c->position.x] == ROOM)
        {
            d->map[c->position.y][c->position.x] = d->terrain[c->position.y][c->position.x];
        }*/

        // move
        next.x = dir.x;
        next.y = dir.y;
    }
    else if (cellHardness(dir.y, dir.x) < 255)
    {
        d->hardness[dir.y][dir.x] -= 85;
        next.x = c->position.x; // stay
        next.y = c->position.y;
    }
    else
    {
        next.x = c->position.x; // stay
        next.y = c->position.y;
    }
}

void monster_next_LOS_tunnel(dungeon_t *d, character_t *c, pair_t next)
{
    pair_t dir;

    dir.y = d->PC->position.y - c->position.y;
    dir.x = d->PC->position.x - c->position.x;

    if (dir.y)
    {
        dir.y /= abs(dir.y);
    }
    if (dir.x)
    {
        dir.x /= abs(dir.x);
    }

    dir.x += next.x;
    dir.y += next.y;

    // tunneller digs at wall by -85 each turn
    if (cellHardness(dir.y, dir.x) <= 85 && cellHardness(dir.y, dir.x) != 255)
    {
        if (cellHardness(dir.y, dir.x))
        {
            d->hardness[dir.y][dir.x] = 0;
            if (d->map[c->position.y][c->position.x] != ROOM)
            {
                d->map[c->position.y][c->position.x] = HALL;
                // update maps
                djikstra_tunnel(d);
                djikstra_non_tunnel(d);
            }
        }

        next.x = dir.x;
        next.y = dir.y;
    }
    else if (cellHardness(dir.y, dir.x) < 255)
    {
        d->hardness[dir.y][dir.x] -= 85;
        next.x = c->position.x; // stay
        next.y = c->position.y;
    }
    else
    {
        d->hardness[dir.y][dir.x] -= 85;

        next.x = c->position.x; // stay
        next.y = c->position.y;
    }
}

void monster_next_telepathic(dungeon_t *d, character_t *c, pair_t next)
{
    int tunnelCost, nonTunnelCost;
    pair_t bestPath = {next.x, next.y}; // ?
    // tunnel
    if (c->mon_character->characteristic & MON_TUNNEL)
    {
        do
        {
            tunnelCost = (d->PC_T[next.y - 1][next.x]) + (d->PC_T[next.y - 1][next.x] / 85);
            bestPath.y = next.y - 1;
            bestPath.x = next.x;

            /* check all directions */

            // down
            if (((d->PC_T[next.y + 1][next.x]) + (d->hardness[next.y + 1][next.x] / 85)) < tunnelCost)
            {
                bestPath.y = next.y + 1;
                bestPath.x = next.x;
            }
            // down right
            if (((d->PC_T[next.y + 1][next.x + 1]) + (d->hardness[next.y + 1][next.x + 1] / 85)) < tunnelCost)
            {
                bestPath.y = next.y + 1;
                bestPath.x = next.x + 1;
            }
            // right
            if (((d->PC_T[next.y][next.x + 1]) + (d->hardness[next.y][next.x + 1] / 85)) < tunnelCost)
            {
                bestPath.y = next.y;
                bestPath.x = next.x + 1;
            }
            // up right
            if (((d->PC_T[next.y - 1][next.x + 1]) + (d->hardness[next.y - 1][next.x + 1] / 85)) < tunnelCost)
            {
                bestPath.y = next.y - 1;
                bestPath.x = next.x + 1;
            }
            // up left
            if (((d->PC_T[next.y - 1][next.x - 1]) + (d->hardness[next.y - 1][next.x - 1] / 85)) < tunnelCost)
            {
                bestPath.y = next.y - 1;
                bestPath.x = next.x - 1;
            }
            // left
            if (((d->PC_T[next.y][next.x - 1]) + (d->hardness[next.y][next.x - 1] / 85)) < tunnelCost)
            {
                bestPath.y = next.y;
                bestPath.x = next.x - 1;
            }
            // down left
            if (((d->PC_T[next.y + 1][next.x - 1]) + (d->hardness[next.y + 1][next.x - 1] / 85)) < tunnelCost)
            {
                bestPath.y = next.y + 1;
                bestPath.x = next.x - 1;
            }
        } while (is_not_occupied_by_monster(d, bestPath.y, bestPath.x));

        // tunneller digs at wall by -85 each turn
        if (cellHardness(bestPath.y, bestPath.x) <= 85 && cellHardness(bestPath.y, bestPath.x) != 255)
        {

            d->hardness[bestPath.y][bestPath.x] = 0;
            if (d->map[c->position.y][c->position.x] != ROOM)
            {
                d->map[c->position.y][c->position.x] = HALL;
                // update maps
                djikstra_tunnel(d);
                djikstra_non_tunnel(d);
            }
            else if (d->map[c->position.y][c->position.x] == ROOM)
            {
                d->map[c->position.y][c->position.x] = d->terrain[c->position.y][c->position.x];
            }

            // move
            next.x = bestPath.x;
            next.y = bestPath.y;
        }
        else if (cellHardness(bestPath.y, bestPath.x) < 255)
        {
            d->hardness[bestPath.y][bestPath.x] -= 85;
            next.x = c->position.x; // stay
            next.y = c->position.y;
        }
        else
        {
            next.x = c->position.x; // stay
            next.y = c->position.y;
        }
    }
    // non tunnel
    else
    {
        bestPath.y = next.y;
        bestPath.x = next.x;
        // up
        nonTunnelCost = d->PC_N[next.y - 1][next.x];
        // down
        if (((d->PC_N[next.y + 1][next.x]) + (d->hardness[next.y + 1][next.x] / 85)) < nonTunnelCost)
        {
            next.y = next.y + 1;
            next.x = next.x;
            return;
        }
        // down right
        if ((d->PC_N[next.y + 1][next.x + 1]) < nonTunnelCost)
        {
            next.y = next.y + 1;
            next.x = next.x + 1;
            return;
        }
        // right
        if ((d->PC_N[next.y][next.x + 1]) < nonTunnelCost)
        {
            next.y = next.y;
            next.x = next.x + 1;
            return;
        }
        // up right
        if ((d->PC_N[next.y - 1][next.x + 1]) < nonTunnelCost)
        {
            next.y = next.y - 1;
            next.x = next.x + 1;
            return;
        }
        // up left
        if ((d->PC_N[next.y - 1][next.x - 1]) < nonTunnelCost)
        {
            next.y = next.y - 1;
            next.x = next.x - 1;
            return;
        }
        // left
        if ((d->PC_N[next.y][next.x - 1]) < nonTunnelCost)
        {
            next.y = next.y;
            next.x = next.x - 1;
            return;
        }
        // down left
        if ((d->PC_N[next.y + 1][next.x - 1]) < nonTunnelCost)
        {
            next.y = next.y + 1;
            next.x = next.x - 1;
            return;
        }
    }
    if (is_not_occupied_by_monster(d, next.y, next.x))
    {
        next.y = bestPath.y;
        next.x = bestPath.x;
    }
}

// static functions so only the file itself can call
// based on MONS_BIT 4 bit 2^4 = 16
// static void
static void monster_next_00(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : not telepathic : not smart */

    // can only meaningly if sees pc
    if (character_see(d, c, d->PC)) // character_see(d, c, d->PC)
    {
        c->mon_character->last_known_PC_pos.y = d->PC->position.y;
        c->mon_character->last_known_PC_pos.x = d->PC->position.x;
        monster_next_LOS(d, c, next);
    }
    else
    {
        monster_next_random(d, c, next);
    }
}
static void monster_next_01(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : not telepathic : smart */
    if (character_see(d, c, d->PC)) // character_see(d, c, d->PC)
    {
        c->mon_character->last_known_PC_pos.y = d->PC->position.y;
        c->mon_character->last_known_PC_pos.x = d->PC->position.x;
        c->mon_character->seen_PC = 1;
        monster_next_LOS(d, c, next);
    }
    // continue to go to last known point
    else if (c->mon_character->seen_PC)
    {
        monster_next_LOS(d, c, next);
    }
    if (c->mon_character->last_known_PC_pos.x != -1 &&
        c->mon_character->last_known_PC_pos.y != -1)
    {
        if ((next.x == c->mon_character->last_known_PC_pos.x) &&
            (next.y == c->mon_character->last_known_PC_pos.y))
        {
            c->mon_character->seen_PC = 0;
        }
    }
}
static void monster_next_02(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : telepathic : not smart */
    c->mon_character->last_known_PC_pos.y = d->PC->position.y;
    c->mon_character->last_known_PC_pos.x = d->PC->position.x;
    monster_next_LOS(d, c, next);
}
static void monster_next_03(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : telepathic : smart */
    monster_next_telepathic(d, c, next);
}
static void monster_next_04(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : not telepathic : not smart */
    if (character_see(d, c, d->PC)) // character_see(d, c, d->PC)
    {
        c->mon_character->last_known_PC_pos.y = d->PC->position.y;
        c->mon_character->last_known_PC_pos.x = d->PC->position.x;
        monster_next_LOS(d, c, next);
    }
    else
    {
        monster_next_random(d, c, next);
    }
}
static void monster_next_05(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : not telepathic : smart */

    // same as 01 but can tunnel
    if (character_see(d, c, d->PC)) // character_see(d, c, d->PC)
    {
        c->mon_character->last_known_PC_pos.y = d->PC->position.y;
        c->mon_character->last_known_PC_pos.x = d->PC->position.x;
        c->mon_character->seen_PC = 1;
        monster_next_LOS_tunnel(d, c, next);
    }
    // continue to go to last known point
    else if (c->mon_character->seen_PC)
    {
        monster_next_LOS_tunnel(d, c, next);
    }

    // reaches last known point, forgets if seen if pc not there

    if (c->mon_character->last_known_PC_pos.x != -1 &&
        c->mon_character->last_known_PC_pos.y != -1)
    {
        if ((next.x == c->mon_character->last_known_PC_pos.x) &&
            (next.y == c->mon_character->last_known_PC_pos.y))
        {
            c->mon_character->seen_PC = 0;
        }
    }
    /*
    if ((next.x == c->mon_character->last_known_PC_pos.x) &&
        (next.y == c->mon_character->last_known_PC_pos.y))
    {
        c->mon_character->seen_PC = 0;
    }*/
}

static void monster_next_06(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : telepathic : not smart */
    c->mon_character->last_known_PC_pos.y = d->PC->position.y;
    c->mon_character->last_known_PC_pos.x = d->PC->position.x;
    monster_next_LOS_tunnel(d, c, next);
}
static void monster_next_07(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : telepathic : smart */
    monster_next_telepathic(d, c, next);
}
static void monster_next_erratic(dungeon_t *d, character_t *c, pair_t next);

void (*monster_move[])(dungeon_t *d, character_t *c, pair_t next) = {
    /* We'll have one function for each combination of bits, so the *
     * order is based on binary counting through the NPC_* bits.    *
     * It could be very easy to mess this up, so be careful.  We'll *
     * name them according to their hex value.                      */
    monster_next_00,
    monster_next_01,
    monster_next_02,
    monster_next_03,
    monster_next_04,
    monster_next_05,
    monster_next_06,
    monster_next_07,
    monster_next_erratic,
    monster_next_erratic,
    monster_next_erratic,
    monster_next_erratic,
    monster_next_erratic,
    monster_next_erratic,
    monster_next_erratic,
    monster_next_erratic,
};

static void monster_next_erratic(dungeon_t *d, character_t *c, pair_t next)
{
    /*                                               erratic */
    if (rand() & 1)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_move[c->mon_character->characteristic & 0x00000007](d, c, next);
    }
}

/* master next function, calls specific move function based on that monster */
void monster_next_position(dungeon_t *d, character_t *c, pair_t next)
{

    next.y = c->position.y;
    next.x = c->position.x;

    monster_move[c->mon_character->characteristic & 0x0000000f](d, c, next);
}

int monsters_number(dungeon_t *d)
{
    return d->num_monsters;
}