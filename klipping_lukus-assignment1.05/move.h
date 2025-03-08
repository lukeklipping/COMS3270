#ifndef MOVE_H
#define MOVE_H

struct dungeon_t;
typedef struct dungeon_t dungeon_t;

struct character_t;
typedef struct character_t character_t;

struct pair_t;
typedef struct pair_t pair_t;

void move_turn_base(dungeon_t *d, uint32_t direction);
void move_character(dungeon_t *d, character_t *c, pair_t new);

int move_pc(dungeon_t *d, int);

#endif