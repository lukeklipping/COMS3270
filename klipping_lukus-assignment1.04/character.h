#ifndef CHARACTER_H
#define CHARACTER_H

// how far monster can see?
#define SIGHT_DISTANCE 15
#include "dungeonGeneration.h"

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
} character_t;

int character_compare(const void *ch1, const void *ch2);
int character_see(dungeon_t *d, character_t *observer, character_t *target);
void character_delete(void *c);

#endif