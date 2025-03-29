#ifndef PC_H
#define PC_H

#include <stdint.h>

#include "dims.h"
#include "dungeon.h"

class pc : public character
{
public:
    terrain_type_t terrain_known[DUNGEON_Y][DUNGEON_X]; // overall seen terrain
    char visible[DUNGEON_Y][DUNGEON_X];                 // close too, adds to terrain_known
};

void pc_delete(pc_t *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
uint32_t pc_in_room(dungeon_t *d, uint32_t room);
void pc_terrain_learn(pc *p, dungeon_t *d);
terrain_type_t pc_terrain_known(pc *p, int y, int x);
int32_t pc_visible(pc *p, int x, int y);
void pc_terrain_init(pc *p);
#endif
