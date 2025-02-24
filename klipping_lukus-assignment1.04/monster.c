#include <stdlib.h>

#include "monster.h"
#include "character.h"
#include "dungeonGeneration.h"
#include "heap.h"

int32_t monster_comp(const void *c1, const void *c2)
{
    int32_t posDifference;
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

        // find location for pos
        do
        {
            room = random_range(0, d->num_rooms - 1);
            pos.y = random_range(d->rooms[room].y, d->rooms[room].y + d->rooms[room].height - 1);
            pos.x = random_range(d->rooms[room].x, d->rooms[room].x + d->rooms[room].width - 1);
        } while (d->character[pos.y][pos.x]);
        // set characteristics
        monster->position.y = pos.y;
        monster->position.x = pos.x;
        d->character[pos.y][pos.x] = monster;
        monster->speed = random_range(MONSTER_MIN_SPEED, MONSTER_MAX_SPEED);
        monster->alive = 1;
        monster->sequence = ++d->num_sequence;

        // monster->mon_character.cha = malloc(sizeof (*monster->mon_character));
        monster->mon_character.characteristic = rand() & 0x0000000f;
        monster->symbol = symbol[random_range(0, 9)];

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

static void move_random(dungeon_t *d, character_t *c, pair_t *new_pos)
{
    new_pos->x = c->position.x + random_range(-1, 1); // Random movement within 1 cell
    new_pos->y = c->position.y + random_range(-1, 1);
}

void move_toward_pc(dungeon_t *d, character_t *c, pair_t *new_pos)
{
    int dy = d->pc.position.y - c->position.y;
    int dx = d->pc.position.x - c->position.x;

    new_pos->y += (dy ? (dy / abs(dy)) : 0);
    new_pos->x += (dx ? (dx / abs(dx)) : 0);
}
void move_toward_known_pc(dungeon_t *d, character_t *c, pair_t *new_pos)
{
    int dy = c->last_known_pc.y - c->position.y;
    int dx = c->last_known_pc.x - c->position.x;

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
}

int monsters_number(dungeon_t *d)
{
    return d->num_monsters;
}