#include <stdlib.h>

#include "monster.h"
#include "dungeonGeneration.h"
#include "character.h"
#include "heap.h"
#include "path.h"

int32_t monster_comp(const void *c1, const void *c2)
{
    character_t *monster1 = *(character_t **)c1;
    character_t *monster2 = *(character_t **)c2;

    // Compare based on sequence number (or other criteria)
    return monster1->sequence - monster2->sequence;
}

void monsters_generate(dungeon_t *d, heap_t *hp)
{
    int i;
    character_t *monster;
    int room;
    pair_t pos;
    const static char symbol[] = "0123456789";

    d->num_monsters = 10;
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
        monster->position.y = pos.y;
        monster->position.x = pos.x;
        d->character[pos.y][pos.x] = monster;
        monster->speed = RANDOM_RANGE(MONSTER_MIN_SPEED, MONSTER_MAX_SPEED);
        monster->alive = 1;
        monster->sequence = ++d->num_sequence;
        monster->pc = NULL;

        // may be a problem
        monster->mon_character = malloc(sizeof(monster->mon_character));

        monster->mon_character->characteristic = rand() & 0x0000000f;
        monster->symbol = symbol[RANDOM_RANGE(0, 9)];

        heap_insert(hp, monster);
    }
}

void monsters_next(dungeon_t *d, character_t *c, pair_t new_pos)
{
    new_pos.y = c->position.y;
    new_pos.x = c->position.x;

    int cTrait = c->mon_character.characteristic;

    if (cTrait & MON_INTELLIGENT)
    {
        if ()
    }
    if (cTrait & MON_TELEPATHY)
    {
    }
    if (cTrait & MON_TUNNEL)
    {
    }
    if (cTrait & MON_ERRATIC)
    {
        if (rand() % 2)
        {
            move_random(d, c, new_pos);
        }
    }
}

// uses djikstra again
static void monster_next_tun(dungeon_t *d, character_t *c, pair_t next)
{
    if (c->mon_character->& MON_TUNNEL)
    {
    }
}

void monster_next_random(dungeon_t *d, character_t *c, pair_t next)
{
    pair_t dir;
    do
    {
        dir.y = c->position.y + RANDOM_RANGE(-1, 1);
        dir.x = c->position.x + RANDOM_RANGE(-1, 1);
    } while (d->map[dir.y][dir.x] != ROOM);

    next.y = dir.y;
    next.x = dir.x;
}

void monster_next_cansee(dungeon_t *d, character_t *c, pair_t next)
{
    pair_t dir;

    dir.y = d->PC.position.y - c->position.y;
    dir.x = d->PC.position.x - c->position.x;

    // normalize direction
    if (dir.y)
    {
        dir.y /= abs(dir.y);
    }
    if (dir.x)
    {
        dir.x /= abs(dir.x);
    }

    if (d->map[next.y + dir.y][next.x + dir.x] >= (ROOM || HALL))
    { // diaganal
        next.x += dir.x;
        next.y += dir.y;
    }
    else if (d->map[next.y][next.x + dir.x] >= (ROOM || HALL))
    { // hori
        next.x += dir.x;
    }
    if (d->map[next.y + dir.y][next.x] >= (ROOM || HALL))
    { // verti
        next.y += dir.y;
    }
}

void monster_next_cansee_tunnel(dungeon_t *d, character_t *c, pair_t next)
{
    pair_t dir;

    dir.y = d->PC.position.y - c->position.y;
    dir.x = d->PC.position.x - c->position.x;

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
    if (cellHardness(dir.y, dir.x) <= 85)
    {
        if (cellHardness(dir.y, dir.x))
        {
            cellHardness(dir.y, dir.x) = 0;
            d->map[dir.y][dir.x] = HALL;
        }
        // update maps
        djikstra_tunnel(d);
        djikstra_non_tunnel(d);
    }
    else
    {
        cellHardness(dir.y, dir.x) -= 85;
    }
}

// static functions so only the file itself can call
// based on MONS_BIT 4 bit 2^4 = 16
// static void
monster_next_00(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : not telepathic : not smart */
}
static void monster_next_01(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : not telepathic : smart */
}
static void monster_next_02(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : telepathic : not smart */
}
static void monster_next_03(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : not tunneling : telepathic : smart */
}
static void monster_next_04(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : not telepathic : not smart */
}
static void monster_next_05(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : not telepathic : smart */
}
static void monster_next_06(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : telepathic : not smart */
}
static void monster_next_07(dungeon_t *d, character_t *c, pair_t next)
{
    /* not erratic : tunneling : telepathic : smart */
}
static void monster_next_08(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic : not tunneling : not telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}
static void monster_next_09(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic : not tunneling : not telepathic : smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}
static void monster_next_0a(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic : not tunneling : telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}
static void monster_next_ob(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic : not tunneling : telepathic : smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}
static void monster_next_0c(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic : tunneling : not telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}
static void monster_next_0d(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic : tunneling : not telepathic : smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}
static void monster_next_0e(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic : tunneling : telepathic : not smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}
static void monster_next_0f(dungeon_t *d, character_t *c, pair_t next)
{
    /* erratic :  tunneling :  telepathic :  smart */
    if (rand() % 2)
    {
        monster_next_random(d, c, next);
    }
    else
    {
    }
}

void (*npc_move_func[])(dungeon_t *d, character_t *c, pair_t next) = {
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
    monster_next_08,
    monster_next_09,
    monster_next_0a,
    monster_next_0b,
    monster_next_0c,
    monster_next_0d,
    monster_next_0e,
    monster_next_0f,
};

void npc_next_pos(dungeon_t *d, character_t *c, pair_t next)
{
    next[dim_y] = c->position[dim_y];
    next[dim_x] = c->position[dim_x];

    npc_move_func[c->npc->characteristics & 0x0000000f](d, c, next);
}
/*

static void move_random(dungeon_t *d, character_t *c, pair_t *new_pos)
{

    do
    {
        int dy = c->position.x + RANDOM_RANGE(-1, 1);
        int dx = c->position.y + RANDOM_RANGE(-1, 1);
    } while (i)
}

void move_toward_pc(dungeon_t *d, character_t *c, pair_t *new_pos)
{
    int dy = d->PC.y - c->position.y;
    int dx = d->PC.x - c->position.x;

    new_pos->y += (dy ? (dy / abs(dy)) : 0);
    new_pos->x += (dx ? (dx / abs(dx)) : 0);
}
void move_toward_known_pc(dungeon_t *d, character_t *c, pair_t new_pos)
{
    int dy = d->PC.y - c->position.y;
    int dx = d->PC.x - c->position.x;

    if (dy)
    {
        dy /= abs(dy);
    }
    if (dx)
    {
        dx /= abs(dx);
    }

    if (d->map[new_pos.y + dy][new_pos.x + dx] >= ROOM)
    {
        new_pos.y += dy;
        new_pos.x += dx;
    }

    new_pos->y += (dy ? (dy / abs(dy)) : 0);
    new_pos->x += (dx ? (dx / abs(dx)) : 0);
}
void move_tunneling(dungeon_t *d, character_t *c, pair_t *new_pos)
{
    // Example logic: Move in the best direction, even if it means breaking walls
    int dy = d->pc.position.y - c->position.y;
    int dx = d->pc.position.x - c->position.x;

    if (dy)
        new_pos->y += (dy / abs(dy));
    if (dx)
        new_pos->x += (dx / abs(dx));
}*/

int monsters_number(dungeon_t *d)
{
    return d->num_monsters;
}