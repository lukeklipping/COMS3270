#include "path.h"
#include "heap.h"
#include "dungeonGeneration.h"
#include "monster.h"
#include "character.h"

#include <stdlib.h>
#include <string.h>

// because pc sequence is 0
static int static_sequence = 1;

/*int32_t monster_comp(const void *c1, const void *c2)
{
    character_t *monster1 = *(character_t **)c1;
    character_t *monster2 = *(character_t **)c2;

    // Compare based on sequence number (or other criteria)
    return monster1->sequence - monster2->sequence;
}*/
void monsters_delete(monster_t *m)
{
    if (m)
    {
        free(m);
    }
}

void monsters_generate(dungeon_t *d)
{
    int i;
    character_t *monster;
    int room;
    pair_t pos;
    const static char symbol[] = "0123456789ABCDEF";

    d->num_monsters = 10;
    for (i = 0; i < d->num_monsters; i++)
    {
        monster = malloc(sizeof(*monster));
        if (!monster)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
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
        monster->sequence = ++static_sequence;
        // monster->pc = NULL;

        monster->mon_character = malloc(sizeof(*monster->mon_character));

        monster->mon_character->characteristic = rand() & 0x0000000f;
        monster->symbol = symbol[monster->mon_character->characteristic];
        monster->mon_character->seen_PC = 0;
        monster->mon_character->last_known_PC_pos.x = -1;
        monster->mon_character->last_known_PC_pos.y = -1;

        d->character[pos.y][pos.x] = monster;

        /*
        printf("Monster: %c at (%d, %d) speed: %d char: %d\n\n",
               monster->symbol, pos.x, pos.y, monster->speed, monster->mon_character->characteristic);
        */
        heap_insert(&d->heap, monster);
    }
}

void monster_next_random(dungeon_t *d, character_t *c, pair_t *next)
{
    pair_t dir;
    do
    {
        dir.y = c->position.y + RANDOM_RANGE(-1, 1);
        dir.x = c->position.x + RANDOM_RANGE(-1, 1);

        if (dir.y < 1 || dir.y >= DUNGEON_X - 1 ||
            dir.x < 1 || dir.x >= DUNGEON_Y - 1)
        {
            continue;
        }

    } while (d->map[dir.y][dir.x] != ROOM);

    next->y = dir.y;
    next->x = dir.x;
}

void monster_next_LOS(dungeon_t *d, character_t *c, pair_t *next)
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
    next->y = c->position.y;
    next->x = c->position.x;
    int new_y = next->y + dir.y;
    int new_x = next->x + dir.x;

    if (new_y >= 0 && new_y < DUNGEON_Y && new_x >= 0 && new_x < DUNGEON_X &&
        (d->map[new_y][new_x] == ROOM || d->map[new_y][new_x] == HALL))
    {
        // Move diagonally if possible
        next->y += dir.y;
        next->x += dir.x;
    }
    else if (new_x >= 0 && new_x < DUNGEON_X && (d->map[next->y][new_x] == ROOM || d->map[next->y][new_x] == HALL))
    {
        // Move horizontally
        next->x += dir.x;
    }
    else if (new_y >= 0 && new_y < DUNGEON_Y && (d->map[new_y][next->x] == ROOM || d->map[new_y][next->x] == HALL))
    {
        // Move vertically
        next->y += dir.y;
    }
}
// uses djikstra again
void monster_next_rand_tunnel(dungeon_t *d, character_t *c, pair_t *next)
{
    pair_t dir;
    do
    {
        dir.y = c->position.y + RANDOM_RANGE(-1, 1);
        dir.x = c->position.x + RANDOM_RANGE(-1, 1);
    } while (d->map[dir.y][dir.x] == IMMUTABLE_WALL || d->hardness[dir.y][dir.x] == 255);

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
        else if (d->map[c->position.y][c->position.x] == ROOM)
        {
            d->map[c->position.y][c->position.x] = d->terrain[c->position.y][c->position.x];
        }

        // move
        next->x = dir.x;
        next->y = dir.y;
    }
    else
    {
        d->hardness[dir.y][dir.x] -= 85;
        next->x = c->position.x; // stay
        next->y = c->position.y;
    }
}

void monster_next_LOS_tunnel(dungeon_t *d, character_t *c, pair_t *next)
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

    dir.x += next->x;
    dir.y += next->y;

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
        else if (d->map[c->position.y][c->position.x] == ROOM)
        {
            d->map[c->position.y][c->position.x] = d->terrain[c->position.y][c->position.x];
        }

        next->x = dir.x;
        next->y = dir.y;
    }
    else
    {

        d->hardness[dir.y][dir.x] -= 85;
        next->x = c->position.x; // stay
        next->y = c->position.y;
    }
}

void monster_next_telepathic(dungeon_t *d, character_t *c, pair_t *next)
{
    int tunnelCost, nonTunnelCost;
    pair_t bestPath = {next->x, next->y}; // ?
    // tunnel
    if (c->mon_character->characteristic & MON_TUNNEL)
    {
        tunnelCost = (d->PC_T[next->y - 1][next->x]) + (d->PC_T[next->y - 1][next->x] / 85);
        bestPath.y = next->y - 1;
        bestPath.x = next->x;

        /* check all directions */

        // down
        if (((d->PC_T[next->y + 1][next->x]) + (d->hardness[next->y + 1][next->x] / 85)) < tunnelCost)
        {
            bestPath.y = next->y + 1;
            bestPath.x = next->x;
        }
        // down right
        if (((d->PC_T[next->y + 1][next->x + 1]) + (d->hardness[next->y + 1][next->x + 1] / 85)) < tunnelCost)
        {
            bestPath.y = next->y + 1;
            bestPath.x = next->x + 1;
        }
        // right
        if (((d->PC_T[next->y][next->x + 1]) + (d->hardness[next->y][next->x + 1] / 85)) < tunnelCost)
        {
            bestPath.y = next->y;
            bestPath.x = next->x + 1;
        }
        // up right
        if (((d->PC_T[next->y - 1][next->x + 1]) + (d->hardness[next->y - 1][next->x + 1] / 85)) < tunnelCost)
        {
            bestPath.y = next->y - 1;
            bestPath.x = next->x + 1;
        }
        // up left
        if (((d->PC_T[next->y - 1][next->x - 1]) + (d->hardness[next->y - 1][next->x - 1] / 85)) < tunnelCost)
        {
            bestPath.y = next->y - 1;
            bestPath.x = next->x - 1;
        }
        // left
        if (((d->PC_T[next->y][next->x - 1]) + (d->hardness[next->y][next->x - 1] / 85)) < tunnelCost)
        {
            bestPath.y = next->y;
            bestPath.x = next->x - 1;
        }
        // down left
        if (((d->PC_T[next->y + 1][next->x - 1]) + (d->hardness[next->y + 1][next->x - 1] / 85)) < tunnelCost)
        {
            bestPath.y = next->y + 1;
            bestPath.x = next->x - 1;
        }

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
            next->x = bestPath.x;
            next->y = bestPath.y;
        }
        else
        {
            // dig
            d->hardness[bestPath.y][bestPath.x] -= 85;
            next->x = c->position.x; // stay
            next->y = c->position.y;
        }
    }
    // non tunnel
    else
    {
        // up
        nonTunnelCost = d->PC_N[next->y - 1][next->x];
        // down
        if (((d->PC_N[next->y + 1][next->x]) + (d->hardness[next->y + 1][next->x] / 85)) < nonTunnelCost)
        {
            bestPath.y = next->y + 1;
            bestPath.x = next->x;
        }
        // down right
        if ((d->PC_N[next->y + 1][next->x + 1]) < nonTunnelCost)
        {
            bestPath.y = next->y + 1;
            bestPath.x = next->x + 1;
        }
        // right
        if ((d->PC_N[next->y][next->x + 1]) < nonTunnelCost)
        {
            bestPath.y = next->y;
            bestPath.x = next->x + 1;
        }
        // up right
        if ((d->PC_N[next->y - 1][next->x + 1]) < nonTunnelCost)
        {
            bestPath.y = next->y - 1;
            bestPath.x = next->x + 1;
        }
        // up left
        if ((d->PC_N[next->y - 1][next->x - 1]) < nonTunnelCost)
        {
            bestPath.y = next->y - 1;
            bestPath.x = next->x - 1;
        }
        // left
        if ((d->PC_N[next->y][next->x - 1]) < nonTunnelCost)
        {
            bestPath.y = next->y;
            bestPath.x = next->x - 1;
        }
        // down left
        if ((d->PC_N[next->y + 1][next->x - 1]) < nonTunnelCost)
        {
            bestPath.y = next->y + 1;
            bestPath.x = next->x - 1;
        }
        next->y = bestPath.y;
        next->x = bestPath.x;
    }
}

// static functions so only the file itself can call
// based on MONS_BIT 4 bit 2^4 = 16
// static void
static void monster_next_00(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : not tunneling : not telepathic : not smart */

    // can only meaningly if sees pc
    if (character_see(d, c, d->PC))
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
static void monster_next_01(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : not tunneling : not telepathic : smart */
    if (character_see(d, c, d->PC))
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
        if ((next->x == c->mon_character->last_known_PC_pos.x) &&
            (next->y == c->mon_character->last_known_PC_pos.y))
        {
            c->mon_character->seen_PC = 0;
        }
    }
}
static void monster_next_02(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : not tunneling : telepathic : not smart */
    c->mon_character->last_known_PC_pos.y = d->PC->position.y;
    c->mon_character->last_known_PC_pos.x = d->PC->position.x;
    monster_next_LOS(d, c, next);
}
static void monster_next_03(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : not tunneling : telepathic : smart */
    monster_next_telepathic(d, c, next);
}
static void monster_next_04(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : tunneling : not telepathic : not smart */
    if (character_see(d, c, d->PC))
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
static void monster_next_05(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : tunneling : not telepathic : smart */

    // same as 01 but can tunnel
    if (character_see(d, c, d->PC))
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
        if ((next->x == c->mon_character->last_known_PC_pos.x) &&
            (next->y == c->mon_character->last_known_PC_pos.y))
        {
            c->mon_character->seen_PC = 0;
        }
    }
    /*
    if ((next->x == c->mon_character->last_known_PC_pos.x) &&
        (next->y == c->mon_character->last_known_PC_pos.y))
    {
        c->mon_character->seen_PC = 0;
    }*/
}

static void monster_next_06(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : tunneling : telepathic : not smart */
    c->mon_character->last_known_PC_pos.y = d->PC->position.y;
    c->mon_character->last_known_PC_pos.x = d->PC->position.x;
    monster_next_LOS_tunnel(d, c, next);
}
static void monster_next_07(dungeon_t *d, character_t *c, pair_t *next)
{
    /* not erratic : tunneling : telepathic : smart */
    monster_next_telepathic(d, c, next);
}
static void monster_next_08(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic : not tunneling : not telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_00(d, c, next);
    }
}
static void monster_next_09(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic : not tunneling : not telepathic : smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_01(d, c, next);
    }
}
static void monster_next_0a(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic : not tunneling : telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_02(d, c, next);
    }
}
static void monster_next_0b(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic : not tunneling : telepathic : smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_03(d, c, next);
    }
}
static void monster_next_0c(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic : tunneling : not telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_04(d, c, next);
    }
}
static void monster_next_0d(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic : tunneling : not telepathic : smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_05(d, c, next);
    }
}
static void monster_next_0e(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic : tunneling : telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_06(d, c, next);
    }
}
static void monster_next_0f(dungeon_t *d, character_t *c, pair_t *next)
{
    /* erratic :  tunneling :  telepathic :  smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
        monster_next_07(d, c, next);
    }
}

/* master next function, calls specific move function based on that monster */
void monster_next_position(dungeon_t *d, character_t *c, pair_t *next)
{
    if (!c)
    {
        fprintf(stderr, "Error: NULL character passed to monster_next_position()\n");
        return;
    }

    if (!c->mon_character)
    {
        fprintf(stderr, "Error: Monster %c at (%d, %d) has NULL characteristics\n",
                c->symbol, c->position.x, c->position.y);
        return;
    }

    next->y = c->position.y;
    next->x = c->position.x;

    switch (c->mon_character->characteristic)
    {
    case 0x00:
        monster_next_00(d, c, next);
        break;
    case 0x01:
        monster_next_01(d, c, next);
        break;
    case 0x02:
        monster_next_02(d, c, next);
        break;
    case 0x03:
        monster_next_03(d, c, next);
        break;
    case 0x04:
        monster_next_04(d, c, next);
        break;
    case 0x05:
        monster_next_05(d, c, next);
        break;
    case 0x06:
        monster_next_06(d, c, next);
        break;
    case 0x07:
        monster_next_07(d, c, next);
        break;
    case 0x08:
        monster_next_08(d, c, next);
        break;
    case 0x09:
        monster_next_09(d, c, next);
        break;
    case 0x0a:
        monster_next_0a(d, c, next);
        break;
    case 0x0b:
        monster_next_0b(d, c, next);
        break;
    case 0x0c:
        monster_next_0c(d, c, next);
        break;
    case 0x0d:
        monster_next_0d(d, c, next);
        break;
    case 0x0e:
        monster_next_0e(d, c, next);
        break;
    case 0x0f:
        monster_next_0f(d, c, next);
        break;
    default:
        printf("error");
        break;
    }
}

int monsters_number(dungeon_t *d)
{
    return d->num_monsters;
}