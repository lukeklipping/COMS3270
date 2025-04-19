#include <cstdlib>
#include <cstring>
#include <ncurses.h>

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"
#include "io.h"
#include "object.h"

int pc::inven_space()
{
  int i;
  for (i = 0; i < 10; i++)
  {
    if (!inventory[i])
      return i;
  }
  // full
  return -1;
}

int pc::drop(object *o)
{
  if (!o)
  {
    io_queue_message("You don't have that item.");
    return -1;
  }
  for (int i = 0; i < 10; i++)
  {
    if (inventory[i] == o)
    {
      delete o;
      inventory[i] = NULL;
      io_queue_message("You have dropped %s", o->get_name());
      return 0;
    }
  }
  return 0;
}

int pc::expunge(object *o)
{
  if (!o)
  {
    io_queue_message("You don't have that item.");
    return -1;
  }
  io_queue_message("You have expunged %s", o->get_name());
  delete o;
  return 0;
}

int pc::pickup(dungeon *d)
{
  if (inven_space() != -1 && objpair(position))
  {
    d->PC->inventory[inven_space()] = objpair(position);
    io_queue_message("You pick up %s", objpair(position)->get_name());
    objpair(position) = NULL;
  }
  else if (inven_space() == -1 && objpair(position))
  {
    io_queue_message("You can't pick up %s, your bag is full",
                     objpair(position)->get_name());
  }
  return 0;
}
int pc::drop(dungeon *d, object *o)
{
  if (!o)
  {
    io_queue_message("Not object in position...");
    return -1;
  }
  else if (objpair(position))
  {
    io_queue_message("You can't drop %s there, object on ground!", o->get_name());
    return -1;
  }
  objpair(position) = o;
  io_queue_message("You have dropped %s", o->get_name());
  return 0;
}
/*
void pc::wear(int i)
{
  if (!inventory[i])
  {
    io_queue_message("You don't have that item.");
    return;
  }
  object *o;
  uint32_t wear_slot = inventory[i]->get_type();
  if (wear_slot > objtype_SCROLL - 1)
  {
    io_queue_message("You can't wear that item.");
    return;
  }
  o = equipment[i];
}*/
void pc::take_off(object *o)
{
  if (!o)
  {
    io_queue_message("Not wearing anything in position...");
    return;
  }
  else if (inven_space() == -1)
  {
    io_queue_message("You can't take off %s, your bag is full", o->get_name());
    return;
  }

  inventory[inven_space()] = o;
  io_queue_message("You have taken off %s", o->get_name());
  return;
}

uint32_t pc_is_alive(dungeon *d)
{
  return d->PC->alive;
}

void place_pc(dungeon *d)
{
  d->PC->position[dim_y] = rand_range(d->rooms->position[dim_y],
                                      (d->rooms->position[dim_y] +
                                       d->rooms->size[dim_y] - 1));
  d->PC->position[dim_x] = rand_range(d->rooms->position[dim_x],
                                      (d->rooms->position[dim_x] +
                                       d->rooms->size[dim_x] - 1));

  pc_init_known_terrain(d->PC);
  pc_observe_terrain(d->PC, d);
}

void config_pc(dungeon *d)
{
  static dice pc_dice(0, 1, 4);

  d->PC = new pc;

  d->PC->symbol = '@';

  place_pc(d);

  d->PC->speed = PC_SPEED;
  d->PC->alive = 1;
  d->PC->sequence_number = 0;
  d->PC->kills[kill_direct] = d->PC->kills[kill_avenged] = 0;
  d->PC->color.push_back(COLOR_WHITE);
  d->PC->damage = &pc_dice;
  d->PC->name = "You";
  d->PC->hp = 100;

  d->character_map[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC;
  int i;
  for (i = 0; i < 12; i++)
  {
    d->PC->equipment[i] = NULL;
  }
  for (i = 0; i < 10; i++)
  {
    d->PC->inventory[i] = NULL;
  }
  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon *d, pair_t dir)
{
  static uint32_t have_seen_corner = 0;
  static uint32_t count = 0;

  dir[dim_y] = dir[dim_x] = 0;

  if (in_corner(d, d->PC))
  {
    if (!count)
    {
      count = 1;
    }
    have_seen_corner = 1;
  }

  /* First, eat anybody standing next to us. */
  if (charxy(d->PC->position[dim_x] - 1, d->PC->position[dim_y] - 1))
  {
    dir[dim_y] = -1;
    dir[dim_x] = -1;
  }
  else if (charxy(d->PC->position[dim_x], d->PC->position[dim_y] - 1))
  {
    dir[dim_y] = -1;
  }
  else if (charxy(d->PC->position[dim_x] + 1, d->PC->position[dim_y] - 1))
  {
    dir[dim_y] = -1;
    dir[dim_x] = 1;
  }
  else if (charxy(d->PC->position[dim_x] - 1, d->PC->position[dim_y]))
  {
    dir[dim_x] = -1;
  }
  else if (charxy(d->PC->position[dim_x] + 1, d->PC->position[dim_y]))
  {
    dir[dim_x] = 1;
  }
  else if (charxy(d->PC->position[dim_x] - 1, d->PC->position[dim_y] + 1))
  {
    dir[dim_y] = 1;
    dir[dim_x] = -1;
  }
  else if (charxy(d->PC->position[dim_x], d->PC->position[dim_y] + 1))
  {
    dir[dim_y] = 1;
  }
  else if (charxy(d->PC->position[dim_x] + 1, d->PC->position[dim_y] + 1))
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
    if (!against_wall(d, d->PC) && ((rand() & 0x111) == 0x111))
    {
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    }
    else
    {
      dir_nearest_wall(d, d->PC, dir);
    }
  }
  else
  {
    /* And after we've been there, let's head toward the center of the map. */
    if (!against_wall(d, d->PC) && ((rand() & 0x111) == 0x111))
    {
      dir[dim_x] = (rand() % 3) - 1;
      dir[dim_y] = (rand() % 3) - 1;
    }
    else
    {
      dir[dim_x] = ((d->PC->position[dim_x] > DUNGEON_X / 2) ? -1 : 1);
      dir[dim_y] = ((d->PC->position[dim_y] > DUNGEON_Y / 2) ? -1 : 1);
    }
  }

  /* Don't move to an unoccupied location if that places us next to a monster */
  if (!charxy(d->PC->position[dim_x] + dir[dim_x],
              d->PC->position[dim_y] + dir[dim_y]) &&
      ((charxy(d->PC->position[dim_x] + dir[dim_x] - 1,
               d->PC->position[dim_y] + dir[dim_y] - 1) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x] - 1,
                d->PC->position[dim_y] + dir[dim_y] - 1) != d->PC)) ||
       (charxy(d->PC->position[dim_x] + dir[dim_x] - 1,
               d->PC->position[dim_y] + dir[dim_y]) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x] - 1,
                d->PC->position[dim_y] + dir[dim_y]) != d->PC)) ||
       (charxy(d->PC->position[dim_x] + dir[dim_x] - 1,
               d->PC->position[dim_y] + dir[dim_y] + 1) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x] - 1,
                d->PC->position[dim_y] + dir[dim_y] + 1) != d->PC)) ||
       (charxy(d->PC->position[dim_x] + dir[dim_x],
               d->PC->position[dim_y] + dir[dim_y] - 1) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x],
                d->PC->position[dim_y] + dir[dim_y] - 1) != d->PC)) ||
       (charxy(d->PC->position[dim_x] + dir[dim_x],
               d->PC->position[dim_y] + dir[dim_y] + 1) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x],
                d->PC->position[dim_y] + dir[dim_y] + 1) != d->PC)) ||
       (charxy(d->PC->position[dim_x] + dir[dim_x] + 1,
               d->PC->position[dim_y] + dir[dim_y] - 1) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x] + 1,
                d->PC->position[dim_y] + dir[dim_y] - 1) != d->PC)) ||
       (charxy(d->PC->position[dim_x] + dir[dim_x] + 1,
               d->PC->position[dim_y] + dir[dim_y]) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x] + 1,
                d->PC->position[dim_y] + dir[dim_y]) != d->PC)) ||
       (charxy(d->PC->position[dim_x] + dir[dim_x] + 1,
               d->PC->position[dim_y] + dir[dim_y] + 1) &&
        (charxy(d->PC->position[dim_x] + dir[dim_x] + 1,
                d->PC->position[dim_y] + dir[dim_y] + 1) != d->PC))))
  {
    dir[dim_x] = dir[dim_y] = 0;
  }

  return 0;
}

uint32_t pc_in_room(dungeon *d, uint32_t room)
{
  if ((room < d->num_rooms) &&
      (d->PC->position[dim_x] >= d->rooms[room].position[dim_x]) &&
      (d->PC->position[dim_x] < (d->rooms[room].position[dim_x] +
                                 d->rooms[room].size[dim_x])) &&
      (d->PC->position[dim_y] >= d->rooms[room].position[dim_y]) &&
      (d->PC->position[dim_y] < (d->rooms[room].position[dim_y] +
                                 d->rooms[room].size[dim_y])))
  {
    return 1;
  }

  return 0;
}

void pc_learn_terrain(pc *p, pair_t pos, terrain_type ter)
{
  p->known_terrain[pos[dim_y]][pos[dim_x]] = ter;
  p->visible[pos[dim_y]][pos[dim_x]] = 1;
}

void pc_reset_visibility(pc *p)
{
  uint32_t y, x;

  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      p->visible[y][x] = 0;
    }
  }
}

terrain_type pc_learned_terrain(pc *p, int16_t y, int16_t x)
{
  if (y < 0 || y >= DUNGEON_Y || x < 0 || x >= DUNGEON_X)
  {
    io_queue_message("Invalid value to %s: %d, %d", __FUNCTION__, y, x);
  }

  return p->known_terrain[y][x];
}

void pc_init_known_terrain(pc *p)
{
  uint32_t y, x;

  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      p->known_terrain[y][x] = ter_unknown;
      p->visible[y][x] = 0;
    }
  }
}

void pc_observe_terrain(pc *p, dungeon *d)
{
  pair_t where;
  int16_t y_min, y_max, x_min, x_max;

  y_min = p->position[dim_y] - PC_VISUAL_RANGE;
  if (y_min < 0)
  {
    y_min = 0;
  }
  y_max = p->position[dim_y] + PC_VISUAL_RANGE;
  if (y_max > DUNGEON_Y - 1)
  {
    y_max = DUNGEON_Y - 1;
  }
  x_min = p->position[dim_x] - PC_VISUAL_RANGE;
  if (x_min < 0)
  {
    x_min = 0;
  }
  x_max = p->position[dim_x] + PC_VISUAL_RANGE;
  if (x_max > DUNGEON_X - 1)
  {
    x_max = DUNGEON_X - 1;
  }

  for (where[dim_y] = y_min; where[dim_y] <= y_max; where[dim_y]++)
  {
    where[dim_x] = x_min;
    can_see(d, p->position, where, 1, 1);
    where[dim_x] = x_max;
    can_see(d, p->position, where, 1, 1);
  }
  /* Take one off the x range because we alreay hit the corners above. */
  for (where[dim_x] = x_min - 1; where[dim_x] <= x_max - 1; where[dim_x]++)
  {
    where[dim_y] = y_min;
    can_see(d, p->position, where, 1, 1);
    where[dim_y] = y_max;
    can_see(d, p->position, where, 1, 1);
  }
}

int32_t is_illuminated(pc *p, int16_t y, int16_t x)
{
  return p->visible[y][x];
}

void pc_see_object(character *the_pc, object *o)
{
  if (o)
  {
    o->has_been_seen();
  }
}
