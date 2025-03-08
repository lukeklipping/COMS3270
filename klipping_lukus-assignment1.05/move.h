#ifndef MOVE_H
#define MOVE_H
#include <stdint.h>

struct dungeon_t;
typedef struct dungeon_t dungeon_t;

struct character_t;
typedef struct character_t character_t;

struct pair_t;
typedef struct pair_t pair_t;

void move_turn_base(dungeon_t *d); // uint32_t direction
void move_character(dungeon_t *d, character_t *c, pair_t new);
void do_combat(dungeon_t *d, character_t *atk, character_t *def);

int move_pc(dungeon_t *d, int);
uint32_t in_corner(dungeon_t *d, character_t *c);
uint32_t against_wall(dungeon_t *d, character_t *c);
void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir);

#endif