#ifndef DUNGEON_H
#define DUNGEON_H

#include "heap.h"
#include "dims.h"
#include "character.h"

#define DUNGEON_X 80
#define DUNGEON_Y 21
#define MIN_ROOMS 6
#define MAX_ROOMS 10
#define ROOM_MIN_X 4
#define ROOM_MIN_Y 3
#define ROOM_MAX_X 20
#define ROOM_MAX_Y 15
#define VISUAL_RANGE 15
#define NPC_VISUAL_RANGE 30
#define PC_SPEED 10
#define PC_VISUAL 20
#define NPC_MIN_SPEED 5
#define NPC_MAX_SPEED 20
#define MAX_MONSTERS 15
#define SAVE_DIR ".rlg327"
#define DUNGEON_SAVE_FILE "dungeon"
#define DUNGEON_SAVE_SEMANTIC "RLG327-" TERM
#define DUNGEON_SAVE_VERSION 0U

#define mappair(pair) (d->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
#define hardnessxy(x, y) (d->hardness[y][x])
#define charpair(pair) (d->characters[pair[dim_y]][pair[dim_x]])
#define charxy(x, y) (d->characters[y][x])

typedef enum __attribute__((__packed__)) terrain_type
{
  ter_debug,
  ter_unknown, // for new seen
  ter_wall,
  ter_wall_immutable,
  ter_floor,
  ter_floor_room,
  ter_floor_hall,
  ter_stairs,
  ter_stairs_up,
  ter_stairs_down
} terrain_type_t;

typedef struct room
{
  pair_t position;
  pair_t size;
} room_t;

#ifdef __cplusplus
class pc;
#else

typedef void pc;
#endif

class dungeon
{
public:
  uint32_t num_rooms;
  room_t *rooms;
  terrain_type_t map[DUNGEON_Y][DUNGEON_X];
  /* Since hardness is usually not used, it would be expensive to pull it *
   * into cache every time we need a map cell, so we store it in a        *
   * parallel array, rather than using a structure to represent the       *
   * cells.  We may want a cell structure later, but from a performanace  *
   * perspective, it would be a bad idea to ever have the map be part of  *
   * that structure.  Pathfinding will require efficient use of the map,  *
   * and pulling in unnecessary data with each map cell would add a lot   *
   * of overhead to the memory system.                                    */
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_distance[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_tunnel[DUNGEON_Y][DUNGEON_X];
  character *characters[DUNGEON_Y][DUNGEON_X];
  pc *thepc;
  heap_t events;
  uint16_t num_monsters;
  uint16_t max_monsters;
  uint32_t character_sequence_number;
  /* Game time isn't strictly necessary.  It's implicit in the turn number *
   * of the most recent thing removed from the event queue; however,       *
   * including it here--and keeping it up to date--provides a measure of   *
   * convenience, e.g., the ability to create a new event without explicit *
   * information from the current event.                                   */
  uint32_t time;
  uint32_t is_new;
  uint32_t quit;
  uint32_t fog_enabled;
};

void init_dungeon(dungeon_t *d);
void new_dungeon(dungeon_t *d);
void delete_dungeon(dungeon_t *d);
int gen_dungeon(dungeon_t *d);
int write_dungeon(dungeon_t *d, char *file);
int read_dungeon(dungeon_t *d, char *file);
int read_pgm(dungeon_t *d, char *pgm);

#endif
