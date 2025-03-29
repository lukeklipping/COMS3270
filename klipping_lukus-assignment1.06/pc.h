#ifndef PC_H
#define PC_H

#include <stdint.h>

#include "character.h"
#include "dims.h"
#include "dungeon.h"

#ifdef __cplusplus
class pc : public character
{
public:
    terrain_type_t terrain_known[DUNGEON_Y][DUNGEON_X]; // overall seen terrain
    unsigned char visible[DUNGEON_Y][DUNGEON_X];        // close too, adds to terrain_known
};
extern "C"
{
#else
typedef void pc;

#endif
#include "dungeon.h"

    // void pc_delete(pc_t *pc);
    uint32_t pc_is_alive(dungeon_t *d);
    void config_pc(dungeon_t *d);
    uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
    void place_pc(dungeon_t *d);
    uint32_t pc_in_room(dungeon_t *d, uint32_t room);
    void pc_terrain_see(pc *p, dungeon_t *d);
    void pc_terrain_learn(pc *p, pair_t ps, terrain_type_t ter);
    terrain_type_t pc_terrain_known(pc *p, int y, int x);
    int32_t pc_visible(pc *p, int x, int y);
    void pc_terrain_init(pc *p);
    void pc_reset_visible(pc *p);

#ifdef __cplusplus
}
#endif
#endif