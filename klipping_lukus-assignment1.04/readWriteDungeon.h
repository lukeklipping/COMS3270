#ifndef READWRITEDUNGEON_H
#define READWRITEDUNGEON_H

#include "dungeonGeneration.h"

int save_stairs(dungeon_t *d, FILE *f);
int count_up_stairs(dungeon_t *d);
int count_down_stairs(dungeon_t *d);
int read_dungeon_map(dungeon_t *d, FILE* f);
int read_rooms(dungeon_t *d, FILE *f);
int read_stairs(dungeon_t *d, FILE *f);
int calculate_dungeon_size(dungeon_t *d);
int load_dungeon(dungeon_t *d, char *file);
int makedirect(char *dir);
int save_dungeon(dungeon_t *d, char *file);

#endif