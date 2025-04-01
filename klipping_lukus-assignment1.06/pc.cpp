#include <stdlib.h>

#include "string.h"

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"

uint32_t pc_is_alive(dungeon_t *d)
{
  return d->thepc->alive;
}

void place_pc(dungeon_t *d)
{
  d->thepc->position[dim_y] = rand_range(d->rooms->position[dim_y],
                                         (d->rooms->position[dim_y] +
                                          d->rooms->size[dim_y] - 1));
  d->thepc->position[dim_x] = rand_range(d->rooms->position[dim_x],
                                         (d->rooms->position[dim_x] +
                                          d->rooms->size[dim_x] - 1));
  pc_terrain_init(d->thepc);
  pc_terrain_see(d->thepc, d);
}

void config_pc(dungeon_t *d)
{
  // memset(&d->thepc, 0, sizeof(d->thepc));
  d->thepc = new pc;
  d->thepc->symbol = '@';

  place_pc(d);

  d->thepc->speed = PC_SPEED;
  d->thepc->alive = 1;
  d->thepc->sequence_number = 0;
  // d->thepc->pc = (pc_t *)calloc(1, sizeof(*d->thepc->pc));
  // d->thepc->npc = NULL;
  d->thepc->kills[kill_direct] = d->thepc->kills[kill_avenged] = 0;

  d->characters[d->thepc->position[dim_y]][d->thepc->position[dim_x]] = d->thepc;

  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  static uint32_t have_seen_corner = 0;
  static uint32_t count = 0;
  static int target_room = -1;
  static int target_is_valid = 0;

  if (target_is_valid &&
      (d->thepc->position[dim_x] == d->rooms[target_room].position[dim_x]) &&
      (d->thepc->position[dim_y] == d->rooms[target_room].position[dim_y]))
  {
    target_is_valid = 0;
  }

  dir[dim_y] = dir[dim_x] = 0;

  if (in_corner(d, d->thepc))
  {
    if (!count)
    {
      count = 1;
    }
    have_seen_corner = 1;
  }

  /* First, eat anybody standing next to us. */
  if (charxy(d->thepc->position[dim_x] - 1, d->thepc->position[dim_y] - 1))
  {
    dir[dim_y] = -1;
    dir[dim_x] = -1;
  }
  else if (charxy(d->thepc->position[dim_x], d->thepc->position[dim_y] - 1))
  {
    dir[dim_y] = -1;
  }
  else if (charxy(d->thepc->position[dim_x] + 1, d->thepc->position[dim_y] - 1))
  {
    dir[dim_y] = -1;
    dir[dim_x] = 1;
  }
  else if (charxy(d->thepc->position[dim_x] - 1, d->thepc->position[dim_y]))
  {
    dir[dim_x] = -1;
  }
  else if (charxy(d->thepc->position[dim_x] + 1, d->thepc->position[dim_y]))
  {
    dir[dim_x] = 1;
  }
  else if (charxy(d->thepc->position[dim_x] - 1, d->thepc->position[dim_y] + 1))
  {
    dir[dim_y] = 1;
    dir[dim_x] = -1;
  }
  else if (charxy(d->thepc->position[dim_x], d->thepc->position[dim_y] + 1))
  {
    dir[dim_y] = 1;
  }
  else if (charxy(d->thepc->position[dim_x] + 1, d->thepc->position[dim_y] + 1))
  {
    dir[dim_y] = 1;
    dir[dim_x] = 1;
  }
  else if (!have_seen_corner || count < 250)
  {
    /* Head to a corner and let most of the NPCs kill each other off */
    if (count)
    {
      count++;
    }
    if (!against_wall(d, d->thepc) && ((rand() & 0x111) == 0x111))
    {
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    }
    else
    {
      dir_nearest_wall(d, d->thepc, dir);
    }
  }
  else
  {
    /* And after we've been there, let's cycle through the rooms, *
     * one-by-one, until the game ends                            */
    if (target_room == -1)
    {
      target_room = 0;
      target_is_valid = 1;
    }
    if (!target_is_valid)
    {
      target_is_valid = 1;
      target_room = (target_room + 1) % d->num_rooms;
    }
    /* When against the dungeon border, always head toward the target; *
     * otherwise, head toward the target with 1/3 probability.         */
    if (against_wall(d, d->thepc) || rand_under(1, 3))
    {
      dir[dim_x] = ((d->thepc->position[dim_x] >
                     d->rooms[target_room].position[dim_x])
                        ? -1
                        : 1);
      dir[dim_y] = ((d->thepc->position[dim_y] >
                     d->rooms[target_room].position[dim_y])
                        ? -1
                        : 1);
    }
    else
    {
      /* Else we'll choose a random direction */
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    }
  }

  /* Don't move to an unoccupied location if that places us next to a monster */
  if (!charxy(d->thepc->position[dim_x] + dir[dim_x],
              d->thepc->position[dim_y] + dir[dim_y]) &&
      ((charxy(d->thepc->position[dim_x] + dir[dim_x] - 1,
               d->thepc->position[dim_y] + dir[dim_y] - 1) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x] - 1,
                d->thepc->position[dim_y] + dir[dim_y] - 1) != d->thepc)) ||
       (charxy(d->thepc->position[dim_x] + dir[dim_x] - 1,
               d->thepc->position[dim_y] + dir[dim_y]) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x] - 1,
                d->thepc->position[dim_y] + dir[dim_y]) != d->thepc)) ||
       (charxy(d->thepc->position[dim_x] + dir[dim_x] - 1,
               d->thepc->position[dim_y] + dir[dim_y] + 1) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x] - 1,
                d->thepc->position[dim_y] + dir[dim_y] + 1) != d->thepc)) ||
       (charxy(d->thepc->position[dim_x] + dir[dim_x],
               d->thepc->position[dim_y] + dir[dim_y] - 1) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x],
                d->thepc->position[dim_y] + dir[dim_y] - 1) != d->thepc)) ||
       (charxy(d->thepc->position[dim_x] + dir[dim_x],
               d->thepc->position[dim_y] + dir[dim_y] + 1) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x],
                d->thepc->position[dim_y] + dir[dim_y] + 1) != d->thepc)) ||
       (charxy(d->thepc->position[dim_x] + dir[dim_x] + 1,
               d->thepc->position[dim_y] + dir[dim_y] - 1) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x] + 1,
                d->thepc->position[dim_y] + dir[dim_y] - 1) != d->thepc)) ||
       (charxy(d->thepc->position[dim_x] + dir[dim_x] + 1,
               d->thepc->position[dim_y] + dir[dim_y]) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x] + 1,
                d->thepc->position[dim_y] + dir[dim_y]) != d->thepc)) ||
       (charxy(d->thepc->position[dim_x] + dir[dim_x] + 1,
               d->thepc->position[dim_y] + dir[dim_y] + 1) &&
        (charxy(d->thepc->position[dim_x] + dir[dim_x] + 1,
                d->thepc->position[dim_y] + dir[dim_y] + 1) != d->thepc))))
  {
    dir[dim_x] = dir[dim_y] = 0;
  }

  return 0;
}

uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
  if ((room < d->num_rooms) &&
      (d->thepc->position[dim_x] >= d->rooms[room].position[dim_x]) &&
      (d->thepc->position[dim_x] < (d->rooms[room].position[dim_x] +
                                    d->rooms[room].size[dim_x])) &&
      (d->thepc->position[dim_y] >= d->rooms[room].position[dim_y]) &&
      (d->thepc->position[dim_y] < (d->rooms[room].position[dim_y] +
                                    d->rooms[room].size[dim_y])))
  {
    return 1;
  }

  return 0;
}

terrain_type_t pc_terrain_known(pc *p, int y, int x)
{
  return p->terrain_known[y][x];
}

int32_t pc_visible(pc *p, int x, int y)
{
  return p->visible[y][x];
}

void pc_terrain_init(pc *p)
{
  int y, x;
  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      p->terrain_known[y][x] = ter_unknown;
      p->visible[y][x] = 0;
    }
  }
}
void pc_terrain_learn(pc *p, pair_t ps, terrain_type_t ter)
{
  p->terrain_known[ps[dim_y]][ps[dim_x]] = ter;
  p->visible[ps[dim_y]][ps[dim_x]] = 1;
}

void pc_terrain_see(pc *p, dungeon_t *d)
{
  pair_t point;
  int ymin, ymax, xmin, xmax;

  ymin = p->position[dim_y] - PC_VISUAL;
  if (ymin < 0)
  {
    ymin = 0;
  }
  ymax = p->position[dim_y] + PC_VISUAL;
  if (ymax > DUNGEON_Y - 1) // account for boundary
  {
    ymax = DUNGEON_Y - 1;
  }

  // x
  xmin = p->position[dim_x] - PC_VISUAL;
  if (xmin < 0)
  {
    xmin = 0;
  }
  xmax = p->position[dim_x] + PC_VISUAL;
  if (xmax > DUNGEON_X - 1) // account for boundary
  {
    xmax = DUNGEON_X - 1;
  }

  for (point[dim_y] = ymin; point[dim_y] <= ymax; point[dim_y]++)
  { // check vert bounds
    point[dim_x] = xmin;
    can_see(d, p->position, point, 1);
    point[dim_x] = xmax;
    can_see(d, p->position, point, 1);
  }
  for (point[dim_x] = xmin - 1; point[dim_x] <= xmax - 1; point[dim_x]++)
  {
    point[dim_y] = ymin;
    can_see(d, p->position, point, 1);
    point[dim_y] = ymax;
    can_see(d, p->position, point, 1);
  }
}

void pc_reset_visible(pc *p)
{
  int y, x;
  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      p->visible[y][x] = 0;
    }
  }
}
