#ifndef CHARACTER_H
#define CHARACTER_H

#include "dungeonGeneration.h"

// how far monster can see?
#define SIGHT_DISTANCE 15

typedef struct dungeon dungeon_t;
typedef struct monster monster_t;

typedef struct
{
    char symbol;
    pair_t position;
    int speed;
    int alive; // al= 1
    int sequence;
    monster_t *mon_character;
    pc_t *pc;
} character_t;

int character_compare(void *ch1, void *ch2);
int character_see(dungeon_t *d, character_t *observer, character_t *target);
void character_delete(void *c);

#endif