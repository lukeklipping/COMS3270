#include <stdlib.h>

#include "character.h"
#include "heap.h"
#include "npc.h"
#include "pc.h"
#include "dungeon.h"
#include "utils.h"

void character_delete(character *v)
{
  delete v;
}

int16_t get_x(const character *c)
{
  return c->position[dim_x];
}
int16_t get_y(const character *c)
{
  return c->position[dim_y];
}

uint32_t can_see(dungeon_t *d, pair_t voyeur, pair_t exhibitionist, uint8_t isPC)
{
  /* Application of Bresenham's Line Drawing Algorithm.  If we can draw *
   * a line from v to e without intersecting any walls, then v can see  *
   * e.  Unfortunately, Bresenham isn't symmetric, so line-of-sight     *
   * based on this approach is not reciprocal (Helmholtz Reciprocity).  *
   * This is a very real problem in roguelike games, and one we're      *
   * going to ignore for now.  Algorithms that are symmetrical are far  *
   * more expensive.                                                    */

  pair_t first, second;
  pair_t del, f;
  int16_t a, b, c, i;
  int16_t vis;

  vis = isPC ? PC_VISUAL : NPC_VISUAL_RANGE;

  first[dim_x] = voyeur[dim_x];
  first[dim_y] = voyeur[dim_y];
  second[dim_x] = exhibitionist[dim_x];
  second[dim_y] = exhibitionist[dim_y];

  if ((abs(first[dim_x] - second[dim_x]) > vis) ||
      (abs(first[dim_y] - second[dim_y]) > vis))
  {
    return 0;
  }

  /*
  mappair(first) = ter_debug;
  mappair(second) = ter_debug;
  */

  if (second[dim_x] > first[dim_x])
  {
    del[dim_x] = second[dim_x] - first[dim_x];
    f[dim_x] = 1;
  }
  else
  {
    del[dim_x] = first[dim_x] - second[dim_x];
    f[dim_x] = -1;
  }

  if (second[dim_y] > first[dim_y])
  {
    del[dim_y] = second[dim_y] - first[dim_y];
    f[dim_y] = 1;
  }
  else
  {
    del[dim_y] = first[dim_y] - second[dim_y];
    f[dim_y] = -1;
  }

  if (del[dim_x] > del[dim_y])
  {
    a = del[dim_y] + del[dim_y];
    c = a - del[dim_x];
    b = c - del[dim_x];
    for (i = 0; i <= del[dim_x]; i++)
    {
      if (isPC)
      { // adds learn terrain
        pc_terrain_learn(d->thepc, first, mappair(first));
      }
      if ((mappair(first) < ter_floor) && i && (i != del[dim_x]))
      {
        return 0;
      }
      /*      mappair(first) = ter_debug;*/
      first[dim_x] += f[dim_x];
      if (c < 0)
      {
        c += a;
      }
      else
      {
        c += b;
        first[dim_y] += f[dim_y];
      }
    }
    return 1;
  }
  else
  {
    a = del[dim_x] + del[dim_x];
    c = a - del[dim_y];
    b = c - del[dim_y];
    for (i = 0; i <= del[dim_y]; i++)
    {
      if (isPC)
      { // adds learn terrain
        pc_terrain_learn(d->thepc, first, mappair(first));
      }
      if ((mappair(first) < ter_floor) && i && (i != del[dim_y]))
      {
        return 0;
      }
      /*      mappair(first) = ter_debug;*/
      first[dim_y] += f[dim_y];
      if (c < 0)
      {
        c += a;
      }
      else
      {
        c += b;
        first[dim_x] += f[dim_x];
      }
    }
    return 1;
  }

  return 1;
}
