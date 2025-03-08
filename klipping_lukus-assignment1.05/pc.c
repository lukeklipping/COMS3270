#include "dungeonGeneration.h"
#include "path.h"
#include "character.h"
#include "pc.h"

/* Deletes PC */
void pc_delete(pc_t *p)
{
    if (p)
    {
        free(p);
        p = NULL;
    }
}

/* Returns whether pc is alive or not */
int pc_alive(dungeon_t *d)
{
    return d->PC->alive;
}

/* Places pc into random spot within first room */
void pc_place(dungeon_t *d)
{
    d->PC->position.y = RANDOM_RANGE(d->rooms->y, (d->rooms->y + d->rooms->height - 1));
    d->PC->position.x = RANDOM_RANGE(d->rooms->x, (d->rooms->x + d->rooms->width - 1));
}

/* Generates pc coords and places on map */
int pc_generate(dungeon_t *d)
{
    d->PC = malloc(sizeof(*d->PC));
    memset(d->PC, 0, sizeof(*d->PC));

    d->PC->alive = 1;
    d->PC->speed = PC_SPEED;
    d->PC->sequence = 0;
    d->PC->mon_character = NULL;
    d->PC->symbol = PLAYER;
    d->PC->pc = calloc(1, sizeof(*d->PC->pc));

    d->character[d->PC->position.y][d->PC->position.x] = d->PC;

    d->map[d->PC->position.y][d->PC->position.x] = PLAYER;

    djikstra_non_tunnel(d);
    djikstra_tunnel(d);

    return 0;
}

/* Returns 1 if pc is in specified room */
int pc_within_room(dungeon_t *d, int room)
{
    if ((room < d->num_rooms) &&
        (d->PC->position.x >= d->rooms[room].x) &&
        (d->PC->position.x < d->rooms[room].x + d->rooms[room].width) &&
        (d->PC->position.y < d->rooms[room].y + d->rooms[room].height) &&
        (d->PC->position.y >= d->rooms[room].y))
    {
        return 1;
    }
    return 0;
}
