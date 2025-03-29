#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdint.h>

#include "dims.h"

typedef struct dungeon dungeon_t;
typedef struct npc npc_t;
typedef struct pc pc_t;
typedef struct dice_t dice_t;

typedef enum kill_type
{
  kill_direct,
  kill_avenged,
  num_kill_types
} kill_type_t;

#ifdef __cplusplus
class character
{
public:
  char symbol;
  pair_t position;
  uint32_t speed;
  uint32_t alive;
  uint32_t sequence_number;
  uint32_t kills[num_kill_types];
};

extern "C"
{
#else
typedef void character;
#endif

  typedef struct dungeon dungeon_t;

  uint32_t compare_characters_by_next_turn(const void *character1, const void *character2);
  uint32_t can_see(dungeon_t *d, pair_t voyeur, pair_t exhibitionist, uint8_t);
  uint32_t can_see(dungeon_t *d, character_t *voyeur, character_t *exhibitionist);
  void character_delete(void *c);

#ifdef __cplusplus
}
#endif
#endif
