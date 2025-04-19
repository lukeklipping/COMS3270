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
#include "npc.h"

void do_combat(dungeon *d, character *atk, character *def)
{
  uint32_t damage = 0;
  if (atk == d->PC)
  {
    int i;
    for (i = 0; i < EQUIPMENT_SIZE; i++)
    {
      if (i == 0 && !d->PC->equipment[i])
      {
        damage += d->PC->damage->roll();
      }
      else if (d->PC->equipment[i])
      {
        damage += d->PC->equipment[i]->roll_dice();
      }
    }
  }
  else // monster atk
  {
    damage = atk->damage->roll();
  }
  if (atk == d->PC)
  {
    io_queue_message("You hit %s for %d damage.", def->name, damage);
  }
  else
  {
    io_queue_message("%s hits you for %d damage.", atk->name, damage);
  }
  def->hp -= damage;
  if (def->hp <= 0)
  {

    def->alive = 0;

    charpair(def->position) = NULL;
    if (def != d->PC)
    {
      d->num_monsters--;
    }
    else // PC
    {
      delete_pc_equipment(d->PC);
      delete_pc_inventory(d->PC);
    }
    character_increment_dkills(atk);
    character_increment_ikills(atk, (character_get_dkills(def) +
                                     character_get_ikills(def)));
  }
}

void move_character(dungeon *d, character *c, pair_t next)
{
  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x])))
  {
    if (d->PC == c || charpair(next) == d->PC)
    {
      do_combat(d, c, charpair(next));
    }
    else
    {
      d->character_map[character_get_y(c)][character_get_x(c)] = NULL;
      character_set_y(c, next[dim_y]);
      character_set_x(c, next[dim_x]);
      d->character_map[character_get_y(c)][character_get_x(c)] = c;
    }
  }
  else
  {
    /* No character in new position. */

    d->character_map[c->position[dim_y]][c->position[dim_x]] = NULL;
    c->position[dim_y] = next[dim_y];
    c->position[dim_x] = next[dim_x];
    d->character_map[c->position[dim_y]][c->position[dim_x]] = c;
  }

  if (c == d->PC)
  {
    pc_reset_visibility(d->PC);
    pc_observe_terrain(d->PC, d);
  }
}

void do_moves(dungeon *d)
{
  pair_t next;
  character *c;
  event *e;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d))
  {
    /* The PC always goes first one a tie, so we don't use new_event().  *
     * We generate one manually so that we can set the PC sequence       *
     * number to zero.                                                   */
    e = (event *)malloc(sizeof(*e));
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
      e->time = d->time + (1000 / d->PC->speed);
    }
    e->sequence = 0;
    e->c = d->PC;
    heap_insert(&d->events, e);
  }

  while (pc_is_alive(d) &&
         (e = (event *)heap_remove_min(&d->events)) &&
         ((e->type != event_character_turn) || (e->c != d->PC)))
  {
    d->time = e->time;
    if (e->type == event_character_turn)
    {
      c = e->c;
    }
    if (!c->alive)
    {
      if (d->character_map[c->position[dim_y]][c->position[dim_x]] == c)
      {
        d->character_map[c->position[dim_y]][c->position[dim_x]] = NULL;
      }
      if (c != d->PC)
      {
        event_delete(e);
      }
      continue;
    }

    npc_next_pos(d, (npc *)c, next);
    move_character(d, (npc *)c, next);

    heap_insert(&d->events, update_event(d, e, 1000 / c->speed));
  }

  io_display(d);
  if (pc_is_alive(d) && e->c == d->PC)
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

void dir_nearest_wall(dungeon *d, character *c, pair_t dir)
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

uint32_t against_wall(dungeon *d, character *c)
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

uint32_t in_corner(dungeon *d, character *c)
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

static void new_dungeon_level(dungeon *d, uint32_t dir)
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

uint32_t move_pc(dungeon *d, uint32_t dir)
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

  next[dim_y] = d->PC->position[dim_y];
  next[dim_x] = d->PC->position[dim_x];

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
    if (mappair(d->PC->position) == ter_stairs_up)
    {
      was_stairs = 1;
      new_dungeon_level(d, '<');
    }
    break;
  case '>':
    if (mappair(d->PC->position) == ter_stairs_down)
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
    move_character(d, d->PC, next);
    d->PC->pickup(d);
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