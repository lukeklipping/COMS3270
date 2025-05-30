#ifndef PC_H
#define PC_H

#include <stdint.h>

#include "dims.h"
#include "character.h"
#include "dungeon.h"
#include "object.h"

class pc : public character
{
public:
  ~pc() {}
  terrain_type known_terrain[DUNGEON_Y][DUNGEON_X];
  uint8_t visible[DUNGEON_Y][DUNGEON_X];

  // carry slots
  object *inventory[10];
  // equipment slots
  object *equipment[12];

  int pickup(dungeon *d);
  int drop(dungeon *d, object *o);
  int wear(int);
  int expunge(object *o);
  int drop(object *o);
  int take_off(object *o);
  int inven_space();
  int refactor();
};

// void pc_delete(pc *pc);
void delete_pc_inventory(pc *p);
void delete_pc_equipment(pc *p);
uint32_t pc_is_alive(dungeon *d);
void config_pc(dungeon *d);
uint32_t pc_next_pos(dungeon *d, pair_t dir);
void place_pc(dungeon *d);
uint32_t pc_in_room(dungeon *d, uint32_t room);
void pc_learn_terrain(pc *p, pair_t pos, terrain_type ter);
terrain_type pc_learned_terrain(pc *p, int16_t y, int16_t x);
void pc_init_known_terrain(pc *p);
void pc_observe_terrain(pc *p, dungeon *d);
int32_t is_illuminated(pc *p, int16_t y, int16_t x);
void pc_reset_visibility(pc *p);

#endif
