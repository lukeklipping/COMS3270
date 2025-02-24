#ifndef MONSTER_H
#define MONSTER_H

// higher num = lower speed
#define MONSTER_MAX_SPEED 5
#define MONSTER_MIN_SPEED 20

typedef struct pair_t pair_t;

typedef struct{

} monster_t;

int monsters_generate(dungeon_t *d);

#endif