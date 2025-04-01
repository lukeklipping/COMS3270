#include "move.h"

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "utils.h"
#include "path.h"
#include "event.h"
#include "io.h"

void do_combat(dungeon_t *d, character *atk, character *def)
{
  int can_see_atk, can_see_def;

  if (def->alive)
  {
    def->alive = 0;
    charpair(def->position) = NULL;

    if (def != d->thepc)
    {
      d->num_monsters--;
    }
  }
  atk->kills[kill_direct]++;
  atk->kills[kill_avenged] += (def->kills[kill_direct] +
                               def->kills[kill_avenged]);

  if (atk == d->thepc)
  {
    io_queue_message("You smite the %c", def->symbol);
    can_see_atk = can_see(d, d->thepc->position, atk->position, 1);
  }
  else
  {
    can_see_atk = can_see(d, d->thepc->position, atk->position, 0);
  }
  can_see_def = can_see(d, d->thepc->position, def->position, 0);

  if (atk != d->thepc && def != d->thepc)
  {
    if (can_see_atk && !can_see_def)
    {
      io_queue_message("The %c callously murders some poor, "
                       "defenseless creature.",
                       atk->symbol);
    }
    if (can_see_def && !can_see_atk)
    {
      io_queue_message("Something kills the helpless %c.", def->symbol);
    }
    if (can_see_atk && can_see_def)
    {
      io_queue_message("You watch in abject horror as the %c "
                       "gruesomely murders the %c!",
                       atk->symbol, def->symbol);
    }
  }
}

void move_character(dungeon_t *d, character *c, pair_t next)
{
  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x])))
  {
    do_combat(d, c, charpair(next));
  }
  else
  {
    /* No character in new position. */

    d->characters[c->position[dim_y]][c->position[dim_x]] = NULL;
    c->position[dim_y] = next[dim_y];
    c->position[dim_x] = next[dim_x];
    d->characters[c->position[dim_y]][c->position[dim_x]] = c;

    if (c == d->thepc)
    {
      // reset visi
      pc_reset_visible((pc *)c);
      pc_terrain_see((pc *)c, d);
    }
  }
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character_t *c;
  event_t *e;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d))
  {
    /* The PC always goes first one a tie, so we don't use new_event().  *
     * We generate one manually so that we can set the PC sequence       *
     * number to zero.                                                   */
    e = (event_t *)malloc(sizeof(*e));
    e->type = event_character_turn;
    /* Hack: New dungeons are marked.  Unmark and ensure PC goes at d->time, *
     * otherwise, monsters get a turn before the PC.                         */
    if (d->is_new)
    {
      d->is_new = 0;
      e->time = d->time;
    }
    else
    {
      e->time = d->time + (1000 / d->thepc->speed);
    }
    e->sequence = 0;
    e->c = d->thepc;
    heap_insert(&d->events, e);
  }

  while (pc_is_alive(d) &&
         (e = (event_t *)heap_remove_min(&d->events)) &&
         ((e->type != event_character_turn) || (e->c != d->thepc)))
  {
    d->time = e->time;
    if (e->type == event_character_turn)
    {
      c = e->c;
    }
    if (!c->alive)
    {
      if (d->characters[c->position[dim_y]][c->position[dim_x]] == c)
      {
        d->characters[c->position[dim_y]][c->position[dim_x]] = NULL;
      }
      if (c != d->thepc)
      {
        event_delete(e);
      }
      continue;
    }

    npc_next_pos(d, (npc *)c, next);
    move_character(d, c, next);

    heap_insert(&d->events, update_event(d, e, 1000 / c->speed));
  }

  io_display(d);
  if (pc_is_alive(d) && e->c == d->thepc)
  {
    c = e->c;
    d->time = e->time;
    /* Kind of kludgey, but because the PC is never in the queue when   *
     * we are outside of this function, the PC event has to get deleted *
     * and recreated every time we leave and re-enter this function.    */
    e->c = NULL;
    event_delete(e);
    io_handle_input(d);
  }
}

void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (c->position[dim_x] != 1 && c->position[dim_x] != DUNGEON_X - 2)
  {
    dir[dim_x] = (c->position[dim_x] > DUNGEON_X - c->position[dim_x] ? 1 : -1);
  }
  if (c->position[dim_y] != 1 && c->position[dim_y] != DUNGEON_Y - 2)
  {
    dir[dim_y] = (c->position[dim_y] > DUNGEON_Y - c->position[dim_y] ? 1 : -1);
  }
}

uint32_t against_wall(dungeon_t *d, character_t *c)
{
  return ((mapxy(c->position[dim_x] - 1,
                 c->position[dim_y]) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x] + 1,
                 c->position[dim_y]) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x],
                 c->position[dim_y] - 1) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x],
                 c->position[dim_y] + 1) == ter_wall_immutable));
}

uint32_t in_corner(dungeon_t *d, character_t *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(c->position[dim_x] - 1,
                          c->position[dim_y]) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x] + 1,
                          c->position[dim_y]) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x],
                          c->position[dim_y] - 1) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x],
                          c->position[dim_y] + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

static void new_dungeon_level(dungeon_t *d, uint32_t dir)
{
  /* Eventually up and down will be independantly meaningful. *
   * For now, simply generate a new dungeon.                  */

  switch (dir)
  {
  case '<':
  case '>':
    new_dungeon(d);
    break;
  default:
    break;
  }
}

uint32_t move_pc(dungeon_t *d, uint32_t dir)
{
  pair_t next;
  uint32_t was_stairs = 0;
  const char *wallmsg[] = {
      "There's a wall in the way.",
      "BUMP!",
      "Ouch!",
      "You stub your toe.",
      "You can't go that way.",
      "You admire the engravings.",
      "Are you drunk?"};

  next[dim_y] = d->thepc->position[dim_y];
  next[dim_x] = d->thepc->position[dim_x];

  switch (dir)
  {
  case 1:
  case 2:
  case 3:
    next[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    next[dim_y]--;
    break;
  }
  switch (dir)
  {
  case 1:
  case 4:
  case 7:
    next[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    next[dim_x]++;
    break;
  case '<':
    if (mappair(d->thepc->position) == ter_stairs_up)
    {
      was_stairs = 1;
      new_dungeon_level(d, '<');
    }
    break;
  case '>':
    if (mappair(d->thepc->position) == ter_stairs_down)
    {
      was_stairs = 1;
      new_dungeon_level(d, '>');
    }
    break;
  }

  if (was_stairs)
  {
    return 0;
  }

  if ((dir != '>') && (dir != '<') && (mappair(next) >= ter_floor))
  {
    move_character(d, d->thepc, next);
    dijkstra(d);
    dijkstra_tunnel(d);

    return 0;
  }
  else if (mappair(next) < ter_floor)
  {
    io_queue_message(wallmsg[rand() % (sizeof(wallmsg) /
                                       sizeof(wallmsg[0]))]);
    io_display(d);
  }

  return 1;
}
