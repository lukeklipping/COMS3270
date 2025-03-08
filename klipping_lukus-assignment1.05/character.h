#ifndef CHARACTER_H
#define CHARACTER_H

// how far monster can see?
#define SIGHT_DISTANCE 20
#include "dungeonGeneration.h"

typedef enum kill_type
{
    kill_direct,
    kill_avenged,
    num_kill_types
} kill_type_t;

/*struct dungeon_t;
typedef struct dungeon_t dungeon_t;*/
struct monster_t;
typedef struct monster_t monster_t;

struct pc_t;
typedef struct pc_t pc_t;

typedef struct character_t
{
    char symbol;
    pair_t position;
    int speed;
    int alive; // al= 1
    int sequence;
    monster_t *mon_character;
    pc_t *pc;
    uint32_t kills[num_kill_types];

} character_t;

int32_t character_compare(const void *ch1, const void *ch2);
int character_see(dungeon_t *d, character_t *observer, character_t *target);
void character_delete(void *c);

#endif