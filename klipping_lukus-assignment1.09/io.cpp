#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>

#include "io.h"
#include "move.h"
#include "path.h"
#include "pc.h"
#include "utils.h"
#include "dungeon.h"
#include "object.h"
#include "npc.h"
#include "character.h"

/* Same ugly hack we did in path.c */
static dungeon *thedungeon;

typedef struct io_message
{
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head)
  {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *)malloc(sizeof(*tmp))))
  {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof(tmp->msg), format, ap);

  va_end(ap);

  if (!io_head)
  {
    io_head = io_tail = tmp;
  }
  else
  {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head)
  {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    if (io_head)
    {
      attron(COLOR_PAIR(COLOR_CYAN));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(COLOR_CYAN));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

void io_display_tunnel(dungeon *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      if (charxy(x, y) == d->PC)
      {
        mvaddch(y + 1, x, charxy(x, y)->symbol);
      }
      else if (hardnessxy(x, y) == 255)
      {
        mvaddch(y + 1, x, '*');
      }
      else
      {
        mvaddch(y + 1, x, '0' + (d->pc_tunnel[y][x] % 10));
      }
    }
  }
  refresh();
}

void io_display_distance(dungeon *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      if (charxy(x, y))
      {
        mvaddch(y + 1, x, charxy(x, y)->symbol);
      }
      else if (hardnessxy(x, y) != 0)
      {
        mvaddch(y + 1, x, ' ');
      }
      else
      {
        mvaddch(y + 1, x, '0' + (d->pc_distance[y][x] % 10));
      }
    }
  }
  refresh();
}

static char hardness_to_char[] =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

void io_display_hardness(dungeon *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      /* Maximum hardness is 255.  We have 62 values to display it, but *
       * we only want one zero value, so we need to cover [1,255] with  *
       * 61 values, which gives us a divisor of 254 / 61 = 4.164.       *
       * Generally, we want to avoid floating point math, but this is   *
       * not gameplay, so we'll make an exception here to get maximal   *
       * hardness display resolution.                                   */
      mvaddch(y + 1, x, (d->hardness[y][x] ? hardness_to_char[1 + (int)((d->hardness[y][x] / 4.2))] : ' '));
    }
  }
  refresh();
}

static void io_redisplay_visible_monsters(dungeon *d)
{
  /* This was initially supposed to only redisplay visible monsters.  After *
   * implementing that (comparitivly simple) functionality and testing, I   *
   * discovered that it resulted to dead monsters being displayed beyond    *
   * their lifetimes.  So it became necessary to implement the function for *
   * everything in the light radius.  In hindsight, it would be better to   *
   * keep a static array of the things in the light radius, generated in    *
   * io_display() and referenced here to accelerate this.  The whole point  *
   * of this is to accelerate the rendering of multi-colored monsters, and  *
   * it is *significantly* faster than that (it eliminates flickering       *
   * artifacts), but it's still significantly slower than it could be.  I   *
   * will revisit this in the future to add the acceleration matrix.        */
  pair_t pos;
  uint32_t color;
  uint32_t illuminated;

  for (pos[dim_y] = -PC_VISUAL_RANGE;
       pos[dim_y] <= PC_VISUAL_RANGE;
       pos[dim_y]++)
  {
    for (pos[dim_x] = -PC_VISUAL_RANGE;
         pos[dim_x] <= PC_VISUAL_RANGE;
         pos[dim_x]++)
    {
      if ((d->PC->position[dim_y] + pos[dim_y] < 0) ||
          (d->PC->position[dim_y] + pos[dim_y] >= DUNGEON_Y) ||
          (d->PC->position[dim_x] + pos[dim_x] < 0) ||
          (d->PC->position[dim_x] + pos[dim_x] >= DUNGEON_X))
      {
        continue;
      }
      if ((illuminated = is_illuminated(d->PC,
                                        d->PC->position[dim_y] + pos[dim_y],
                                        d->PC->position[dim_x] + pos[dim_x])))
      {
        attron(A_BOLD);
      }
      if (d->character_map[d->PC->position[dim_y] + pos[dim_y]]
                          [d->PC->position[dim_x] + pos[dim_x]] &&
          can_see(d, d->PC->position,
                  d->character_map[d->PC->position[dim_y] + pos[dim_y]]
                                  [d->PC->position[dim_x] +
                                   pos[dim_x]]
                                      ->position,
                  1, 0))
      {
        attron(COLOR_PAIR((color = d->character_map[d->PC->position[dim_y] +
                                                    pos[dim_y]]
                                                   [d->PC->position[dim_x] +
                                                    pos[dim_x]]
                                                       ->get_color())));
        mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                d->PC->position[dim_x] + pos[dim_x],
                character_get_symbol(d->character_map[d->PC->position[dim_y] +
                                                      pos[dim_y]]
                                                     [d->PC->position[dim_x] +
                                                      pos[dim_x]]));
        attroff(COLOR_PAIR(color));
      }
      else if (d->objmap[d->PC->position[dim_y] + pos[dim_y]]
                        [d->PC->position[dim_x] + pos[dim_x]] &&
               (can_see(d, d->PC->position,
                        d->objmap[d->PC->position[dim_y] + pos[dim_y]]
                                 [d->PC->position[dim_x] +
                                  pos[dim_x]]
                                     ->get_position(),
                        1, 0) ||
                d->objmap[d->PC->position[dim_y] + pos[dim_y]]
                         [d->PC->position[dim_x] + pos[dim_x]]
                             ->have_seen()))
      {
        attron(COLOR_PAIR(d->objmap[d->PC->position[dim_y] + pos[dim_y]]
                                   [d->PC->position[dim_x] +
                                    pos[dim_x]]
                                       ->get_color()));
        mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                d->PC->position[dim_x] + pos[dim_x],
                d->objmap[d->PC->position[dim_y] + pos[dim_y]]
                         [d->PC->position[dim_x] + pos[dim_x]]
                             ->get_symbol());
        attroff(COLOR_PAIR(d->objmap[d->PC->position[dim_y] + pos[dim_y]]
                                    [d->PC->position[dim_x] +
                                     pos[dim_x]]
                                        ->get_color()));
      }
      else
      {
        switch (pc_learned_terrain(d->PC,
                                   d->PC->position[dim_y] + pos[dim_y],
                                   d->PC->position[dim_x] +
                                       pos[dim_x]))
        {
        case ter_wall:
        case ter_wall_immutable:
        case ter_unknown:
          mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                  d->PC->position[dim_x] + pos[dim_x], ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                  d->PC->position[dim_x] + pos[dim_x], '.');
          break;
        case ter_floor_hall:
          mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                  d->PC->position[dim_x] + pos[dim_x], '#');
          break;
        case ter_debug:
          mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                  d->PC->position[dim_x] + pos[dim_x], '*');
          break;
        case ter_stairs_up:
          mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                  d->PC->position[dim_x] + pos[dim_x], '<');
          break;
        case ter_stairs_down:
          mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                  d->PC->position[dim_x] + pos[dim_x], '>');
          break;
        default:
          /* Use zero as an error symbol, since it stands out somewhat, and it's *
           * not otherwise used.                                                 */
          mvaddch(d->PC->position[dim_y] + pos[dim_y] + 1,
                  d->PC->position[dim_x] + pos[dim_x], '0');
        }
      }
      attroff(A_BOLD);
    }
  }

  refresh();
}

static int compare_monster_distance(const void *v1, const void *v2)
{
  const character *const *c1 = (const character *const *)v1;
  const character *const *c2 = (const character *const *)v2;

  return (thedungeon->pc_distance[(*c1)->position[dim_y]]
                                 [(*c1)->position[dim_x]] -
          thedungeon->pc_distance[(*c2)->position[dim_y]]
                                 [(*c2)->position[dim_x]]);
}

static character *io_nearest_visible_monster(dungeon *d)
{
  character **c, *n;
  uint32_t x, y, count, i;

  c = (character **)malloc(d->num_monsters * sizeof(*c));

  /* Get a linear list of monsters */
  for (count = 0, y = 1; y < DUNGEON_Y - 1; y++)
  {
    for (x = 1; x < DUNGEON_X - 1; x++)
    {
      if (d->character_map[y][x] && d->character_map[y][x] != d->PC)
      {
        c[count++] = d->character_map[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  thedungeon = d;
  qsort(c, count, sizeof(*c), compare_monster_distance);

  for (n = NULL, i = 0; i < count; i++)
  {
    if (can_see(d, character_get_pos(d->PC), character_get_pos(c[i]), 1, 0))
    {
      n = c[i];
      break;
    }
  }

  free(c);

  return n;
}

void io_display(dungeon *d)
{
  pair_t pos;
  uint32_t illuminated;
  uint32_t color;
  character *c;
  int32_t visible_monsters;

  clear();
  for (visible_monsters = -1, pos[dim_y] = 0;
       pos[dim_y] < DUNGEON_Y;
       pos[dim_y]++)
  {
    for (pos[dim_x] = 0; pos[dim_x] < DUNGEON_X; pos[dim_x]++)
    {
      if ((illuminated = is_illuminated(d->PC,
                                        pos[dim_y],
                                        pos[dim_x])))
      {
        attron(A_BOLD);
      }
      if (d->character_map[pos[dim_y]]
                          [pos[dim_x]] &&
          can_see(d,
                  character_get_pos(d->PC),
                  character_get_pos(d->character_map[pos[dim_y]]
                                                    [pos[dim_x]]),
                  1, 0))
      {
        visible_monsters++;
        attron(COLOR_PAIR((color = d->character_map[pos[dim_y]]
                                                   [pos[dim_x]]
                                                       ->get_color())));
        mvaddch(pos[dim_y] + 1, pos[dim_x],
                character_get_symbol(d->character_map[pos[dim_y]]
                                                     [pos[dim_x]]));
        attroff(COLOR_PAIR(color));
      }
      else if (d->objmap[pos[dim_y]]
                        [pos[dim_x]] &&
               (d->objmap[pos[dim_y]]
                         [pos[dim_x]]
                             ->have_seen() ||
                can_see(d, character_get_pos(d->PC), pos, 1, 0)))
      {
        attron(COLOR_PAIR(d->objmap[pos[dim_y]]
                                   [pos[dim_x]]
                                       ->get_color()));
        mvaddch(pos[dim_y] + 1, pos[dim_x],
                d->objmap[pos[dim_y]]
                         [pos[dim_x]]
                             ->get_symbol());
        attroff(COLOR_PAIR(d->objmap[pos[dim_y]]
                                    [pos[dim_x]]
                                        ->get_color()));
      }
      else
      {
        switch (pc_learned_terrain(d->PC,
                                   pos[dim_y],
                                   pos[dim_x]))
        {
        case ter_wall:
        case ter_wall_immutable:
        case ter_unknown:
          mvaddch(pos[dim_y] + 1, pos[dim_x], ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '.');
          break;
        case ter_floor_hall:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '#');
          break;
        case ter_debug:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '*');
          break;
        case ter_stairs_up:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '<');
          break;
        case ter_stairs_down:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '>');
          break;
        default:
          /* Use zero as an error symbol, since it stands out somewhat, and it's *
           * not otherwise used.                                                 */
          mvaddch(pos[dim_y] + 1, pos[dim_x], '0');
        }
      }
      if (illuminated)
      {
        attroff(A_BOLD);
      }
    }
  }

  mvprintw(23, 1, "PC position is (%2d,%2d).",
           d->PC->position[dim_x], d->PC->position[dim_y]);
  mvprintw(22, 1, "%d known %s.", visible_monsters,
           visible_monsters > 1 ? "monsters" : "monster");
  mvprintw(22, 30, "Nearest visible monster: ");
  mvprintw(23, 30, "PC HP: ");
  attron(COLOR_PAIR(COLOR_GREEN));
  mvprintw(23, 38, "%d/%d", d->PC->hp, PC_HEALTH);
  attroff(COLOR_PAIR(COLOR_GREEN));
  if ((c = io_nearest_visible_monster(d)))
  {
    attron(COLOR_PAIR(COLOR_RED));
    mvprintw(22, 55, "%c at %d %c by %d %c.",
             c->symbol,
             abs(c->position[dim_y] - d->PC->position[dim_y]),
             ((c->position[dim_y] - d->PC->position[dim_y]) <= 0 ? 'N' : 'S'),
             abs(c->position[dim_x] - d->PC->position[dim_x]),
             ((c->position[dim_x] - d->PC->position[dim_x]) <= 0 ? 'W' : 'E'));
    attroff(COLOR_PAIR(COLOR_RED));
  }
  else
  {
    attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(COLOR_BLUE));
  }

  io_print_message_queue(0, 0);

  refresh();
}

static void io_redisplay_non_terrain(dungeon *d, pair_t cursor)
{
  /* For the wiz-mode teleport, in order to see color-changing effects. */
  pair_t pos;
  uint32_t color;
  uint32_t illuminated;

  for (pos[dim_y] = 0; pos[dim_y] < DUNGEON_Y; pos[dim_y]++)
  {
    for (pos[dim_x] = 0; pos[dim_x] < DUNGEON_X; pos[dim_x]++)
    {
      if ((illuminated = is_illuminated(d->PC,
                                        pos[dim_y],
                                        pos[dim_x])))
      {
        attron(A_BOLD);
      }
      if (cursor[dim_y] == pos[dim_y] && cursor[dim_x] == pos[dim_x])
      {
        mvaddch(pos[dim_y] + 1, pos[dim_x], '*');
      }
      else if (d->character_map[pos[dim_y]][pos[dim_x]])
      {
        attron(COLOR_PAIR((color = d->character_map[pos[dim_y]]
                                                   [pos[dim_x]]
                                                       ->get_color())));
        mvaddch(pos[dim_y] + 1, pos[dim_x],
                character_get_symbol(d->character_map[pos[dim_y]][pos[dim_x]]));
        attroff(COLOR_PAIR(color));
      }
      else if (d->objmap[pos[dim_y]][pos[dim_x]])
      {
        attron(COLOR_PAIR(d->objmap[pos[dim_y]][pos[dim_x]]->get_color()));
        mvaddch(pos[dim_y] + 1, pos[dim_x],
                d->objmap[pos[dim_y]][pos[dim_x]]->get_symbol());
        attroff(COLOR_PAIR(d->objmap[pos[dim_y]][pos[dim_x]]->get_color()));
      }
      attroff(A_BOLD);
    }
  }

  refresh();
}

void io_display_no_fog(dungeon *d)
{
  uint32_t y, x;
  uint32_t color;
  character *c;

  clear();
  for (y = 0; y < DUNGEON_Y; y++)
  {
    for (x = 0; x < DUNGEON_X; x++)
    {
      if (d->character_map[y][x])
      {
        attron(COLOR_PAIR((color = d->character_map[y][x]->get_color())));
        mvaddch(y + 1, x, character_get_symbol(d->character_map[y][x]));
        attroff(COLOR_PAIR(color));
      }
      else if (d->objmap[y][x])
      {
        attron(COLOR_PAIR(d->objmap[y][x]->get_color()));
        mvaddch(y + 1, x, d->objmap[y][x]->get_symbol());
        attroff(COLOR_PAIR(d->objmap[y][x]->get_color()));
      }
      else
      {
        switch (mapxy(x, y))
        {
        case ter_wall:
        case ter_wall_immutable:
          mvaddch(y + 1, x, ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(y + 1, x, '.');
          break;
        case ter_floor_hall:
          mvaddch(y + 1, x, '#');
          break;
        case ter_debug:
          mvaddch(y + 1, x, '*');
          break;
        case ter_stairs_up:
          mvaddch(y + 1, x, '<');
          break;
        case ter_stairs_down:
          mvaddch(y + 1, x, '>');
          break;
        default:
          /* Use zero as an error symbol, since it stands out somewhat, and it's *
           * not otherwise used.                                                 */
          mvaddch(y + 1, x, '0');
        }
      }
    }
  }

  mvprintw(23, 1, "PC position is (%2d,%2d).",
           d->PC->position[dim_x], d->PC->position[dim_y]);
  mvprintw(22, 1, "%d %s.", d->num_monsters,
           d->num_monsters > 1 ? "monsters" : "monster");
  mvprintw(22, 30, "Nearest visible monster: ");
  mvprintw(23, 30, "PC HP: %d/%d", d->PC->hp, PC_HEALTH);
  if ((c = io_nearest_visible_monster(d)))
  {
    attron(COLOR_PAIR(COLOR_RED));
    mvprintw(22, 55, "%c at %d %c by %d %c.",
             c->symbol,
             abs(c->position[dim_y] - d->PC->position[dim_y]),
             ((c->position[dim_y] - d->PC->position[dim_y]) <= 0 ? 'N' : 'S'),
             abs(c->position[dim_x] - d->PC->position[dim_x]),
             ((c->position[dim_x] - d->PC->position[dim_x]) <= 0 ? 'W' : 'E'));
    attroff(COLOR_PAIR(COLOR_RED));
  }
  else
  {
    attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(COLOR_BLUE));
  }

  io_print_message_queue(0, 0);

  refresh();
}

void io_display_monster_list(dungeon *d)
{
  mvprintw(11, 33, " HP:    XXXXX ");
  mvprintw(12, 33, " Speed: XXXXX ");
  mvprintw(14, 27, " Hit any key to continue. ");
  refresh();
  getch();
}

uint32_t io_teleport_pc(dungeon *d)
{
  pair_t dest;
  int c;
  fd_set readfs;
  struct timeval tv;

  pc_reset_visibility(d->PC);
  io_display_no_fog(d);

  mvprintw(0, 0,
           "Choose a location.  'g' or '.' to teleport to; 'r' for random.");

  dest[dim_y] = d->PC->position[dim_y];
  dest[dim_x] = d->PC->position[dim_x];

  mvaddch(dest[dim_y] + 1, dest[dim_x], '*');
  refresh();

  do
  {
    do
    {
      FD_ZERO(&readfs);
      FD_SET(STDIN_FILENO, &readfs);

      tv.tv_sec = 0;
      tv.tv_usec = 125000; /* An eigth of a second */

      io_redisplay_non_terrain(d, dest);
    } while (!select(STDIN_FILENO + 1, &readfs, NULL, NULL, &tv));
    /* Can simply draw the terrain when we move the cursor away, *
     * because if it is a character or object, the refresh       *
     * function will fix it for us.                              */
    switch (mappair(dest))
    {
    case ter_wall:
    case ter_wall_immutable:
    case ter_unknown:
      mvaddch(dest[dim_y] + 1, dest[dim_x], ' ');
      break;
    case ter_floor:
    case ter_floor_room:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '.');
      break;
    case ter_floor_hall:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '#');
      break;
    case ter_debug:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '*');
      break;
    case ter_stairs_up:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '<');
      break;
    case ter_stairs_down:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '>');
      break;
    default:
      /* Use zero as an error symbol, since it stands out somewhat, and it's *
       * not otherwise used.                                                 */
      mvaddch(dest[dim_y] + 1, dest[dim_x], '0');
    }
    switch ((c = getch()))
    {
    case '7':
    case 'y':
    case KEY_HOME:
      if (dest[dim_y] != 1)
      {
        dest[dim_y]--;
      }
      if (dest[dim_x] != 1)
      {
        dest[dim_x]--;
      }
      break;
    case '8':
    case 'k':
    case KEY_UP:
      if (dest[dim_y] != 1)
      {
        dest[dim_y]--;
      }
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (dest[dim_y] != 1)
      {
        dest[dim_y]--;
      }
      if (dest[dim_x] != DUNGEON_X - 2)
      {
        dest[dim_x]++;
      }
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      if (dest[dim_x] != DUNGEON_X - 2)
      {
        dest[dim_x]++;
      }
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      if (dest[dim_y] != DUNGEON_Y - 2)
      {
        dest[dim_y]++;
      }
      if (dest[dim_x] != DUNGEON_X - 2)
      {
        dest[dim_x]++;
      }
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      if (dest[dim_y] != DUNGEON_Y - 2)
      {
        dest[dim_y]++;
      }
      break;
    case '1':
    case 'b':
    case KEY_END:
      if (dest[dim_y] != DUNGEON_Y - 2)
      {
        dest[dim_y]++;
      }
      if (dest[dim_x] != 1)
      {
        dest[dim_x]--;
      }
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      if (dest[dim_x] != 1)
      {
        dest[dim_x]--;
      }
      break;
    }
  } while (c != 'g' && c != '.' && c != 'r');

  if (c == 'r')
  {
    do
    {
      dest[dim_x] = rand_range(1, DUNGEON_X - 2);
      dest[dim_y] = rand_range(1, DUNGEON_Y - 2);
    } while (charpair(dest) || mappair(dest) < ter_floor);
  }

  if (charpair(dest) && charpair(dest) != d->PC)
  {
    io_queue_message("Teleport failed.  Destination occupied.");
  }
  else
  {
    d->character_map[d->PC->position[dim_y]][d->PC->position[dim_x]] = NULL;
    d->character_map[dest[dim_y]][dest[dim_x]] = d->PC;

    d->PC->position[dim_y] = dest[dim_y];
    d->PC->position[dim_x] = dest[dim_x];
  }

  pc_observe_terrain(d->PC, d);
  dijkstra(d);
  dijkstra_tunnel(d);

  io_display(d);

  return 0;
}

/* Adjectives to describe our monsters */
static const char *adjectives[] = {
    "A menacing ",
    "A threatening ",
    "A horrifying ",
    "An intimidating ",
    "An aggressive ",
    "A frightening ",
    "A terrifying ",
    "A terrorizing ",
    "An alarming ",
    "A dangerous ",
    "A glowering ",
    "A glaring ",
    "A scowling ",
    "A chilling ",
    "A scary ",
    "A creepy ",
    "An eerie ",
    "A spooky ",
    "A slobbering ",
    "A drooling ",
    "A horrendous ",
    "An unnerving ",
    "A cute little ",  /* Even though they're trying to kill you, */
    "A teeny-weenie ", /* they can still be cute!                 */
    "A fuzzy ",
    "A fluffy white ",
    "A kawaii ",      /* For our otaku */
    "Hao ke ai de ",  /* And for our Chinese */
    "Eine liebliche " /* For our Deutch */
                      /* And there's one special case (see below) */
};

static void io_scroll_monster_list(char (*s)[66], uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1)
  {
    for (i = 0; i < 13; i++)
    {
      mvprintw(i + 6, 9, " %-66s ", s[i + offset]);
    }
    switch (getch())
    {
    case KEY_UP:
      if (offset)
      {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13))
      {
        offset++;
      }
      break;
    case 27:
      return;
    }
  }
}

static bool is_vowel(const char c)
{
  return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
          c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
}

static void io_list_monsters_display(dungeon *d,
                                     character **c,
                                     uint32_t count)
{
  uint32_t i;
  char (*s)[66]; /* pointer to array of 66 char */
  char tmp[41];  /* 19 bytes for relative direction leaves 40 bytes *
                  * for the monster's name (and one for null).      */

  (void)adjectives;

  s = (char (*)[66])malloc((count + 1) * sizeof(*s));

  mvprintw(3, 9, " %-66s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], 66, "You know of %d monsters:", count);
  mvprintw(4, 9, " %-66s ", s[0]);
  mvprintw(5, 9, " %-66s ", "");

  for (i = 0; i < count; i++)
  {
    snprintf(tmp, 41, "%3s%s (%c): ",
             (is_unique(c[i]) ? "" : (is_vowel(character_get_name(c[i])[0]) ? "An " : "A ")),
             character_get_name(c[i]),
             character_get_symbol(c[i]));
    /* These pragma's suppress a "format truncation" warning from gcc. *
     * Stumbled upon a GCC bug when updating monster lists for 1.08.   *
     * Bug is known:                                                   *
     *    https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78969           *
     * GCC calculates a maximum length for the output string under the *
     * assumption that the int conversions can be 11 digits long (-2.1 *
     * billion).  The ints below can never be more than 2 digits.      *
     * Tried supressing the warning by taking the ints mod 100, but    *
     * GCC wasn't smart enough for that, so using a pragma instead.    */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
    snprintf(s[i], 66, "%40s%2d %s by %2d %s", tmp,
             abs(character_get_y(c[i]) - character_get_y(d->PC)),
             ((character_get_y(c[i]) - character_get_y(d->PC)) <= 0 ? "North" : "South"),
             abs(character_get_x(c[i]) - character_get_x(d->PC)),
             ((character_get_x(c[i]) - character_get_x(d->PC)) <= 0 ? "West" : "East"));
#pragma GCC diagnostic pop
    if (count <= 13)
    {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 9, " %-66s ", s[i]);
    }
  }

  if (count <= 13)
  {
    mvprintw(count + 6, 9, " %-66s ", "");
    mvprintw(count + 7, 9, " %-66s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  }
  else
  {
    mvprintw(19, 9, " %-66s ", "");
    mvprintw(20, 9, " %-66s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_monster_list(s, count);
  }

  free(s);
}

static void io_list_monsters(dungeon *d)
{
  character **c;
  uint32_t x, y, count;

  c = (character **)malloc(d->num_monsters * sizeof(*c));

  /* Get a linear list of monsters */
  for (count = 0, y = 1; y < DUNGEON_Y - 1; y++)
  {
    for (x = 1; x < DUNGEON_X - 1; x++)
    {
      if (d->character_map[y][x] && d->character_map[y][x] != d->PC &&
          can_see(d, character_get_pos(d->PC),
                  character_get_pos(d->character_map[y][x]), 1, 0))
      {
        c[count++] = d->character_map[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  thedungeon = d;
  qsort(c, count, sizeof(*c), compare_monster_distance);

  /* Display it */
  io_list_monsters_display(d, c, count);
  free(c);

  /* And redraw the dungeon */
  io_display(d);
}

void io_drop(dungeon *d)
{
  int32_t key;
  int i;
  mvprintw(2, 1, "|-------------------Drop Mode--------------------|");
  for (i = 0; i < 10; i++)
  {
    if (d->PC->inventory[i])
    {
      mvprintw(i + 3, 1, "|%c. %-45s|", '0' + i, d->PC->inventory[i]->get_name());
    }
    else
      mvprintw(i + 3, 1, "|%c. Empty                                        |", '0' + i);
  }
  mvprintw(13, 1, "|------------------------------------------------|");
  mvprintw(14, 1, "|------------------------------------------------|");
  mvprintw(15, 1, "|Choose an object to drop                        |");
  mvprintw(16, 1, "|Enter esc to abort                              |");
  mvprintw(17, 1, "|------------------------------------------------|");
  do
  {
    if ((key = getch()) == 27 /* ESC */)
    {

      io_display(d);
      return;
    }
    else if (key != '0' && key != '1' && key != '2' && key != '3' && key != '4' && key != '5' && key != '6' && key != '7' && key != '8' && key != '9')
    {
      mvprintw(18, 1, "Wrong number!");
      continue;
    }
    else if (d->PC->drop(d, d->PC->inventory[key - '0']))
    {
      io_display(d);
      return;
    }
    else
    {
      d->PC->inventory[key - '0'] = NULL;
      io_display(d);
      return;
    }
  } while (1);
}
void io_inventory(dungeon *d)
{
  int key, i;
  attron(COLOR_PAIR(COLOR_CYAN));
  mvprintw(2, 20, "|%-*s|", MODE_WIDTH, "Inventory");
  attroff(COLOR_PAIR(COLOR_CYAN));

  for (i = 0; i < INVENTORY_SIZE; i++)
  {
    if (d->PC->inventory[i])
      mvprintw(i + 3, 20, "|%2d: %-*s|", i, MODE_WIDTH - 4, d->PC->inventory[i]->get_name());
    else
      mvprintw(i + 3, 20, "|%-*s|", MODE_WIDTH - 4, "EMPTY");
  }

  mvprintw(16, 20, "Enter esc to abort");

  refresh();

  do
  {
    if ((key = getch()) == 27)
    {
      io_display(d);
      return;
    }
  } while (1);
}
void io_equipment(dungeon *d)
{
  int key, i;
  attron(COLOR_PAIR(COLOR_CYAN));
  mvprintw(2, 20, "|-------------Equipment-------------|");
  attroff(COLOR_PAIR(COLOR_CYAN));

  for (i = 0; i < EQUIPMENT_SIZE; i++)
  {
    if (d->PC->equipment[i])
    {
      switch (i)
      {
      case 0:
        mvprintw(i + 3, 20, "|a. WEAPON: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 1:
        mvprintw(i + 3, 20, "|b. OFFHAND: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 2:
        mvprintw(i + 3, 20, "|c. RANGED: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 3:
        mvprintw(i + 3, 20, "|d. ARMOR: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 4:
        mvprintw(i + 3, 20, "|e. HELMET: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 5:
        mvprintw(i + 3, 20, "|f. CLOAK: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 6:
        mvprintw(i + 3, 20, "|g. GLOVES: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 7:
        mvprintw(i + 3, 20, "|h. BOOTS: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 8:
        mvprintw(i + 3, 20, "|i. AMULET: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 9:
        mvprintw(i + 3, 20, "|j. LIGHT: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 10:
        mvprintw(i + 3, 20, "|k. RING1: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 11:
        mvprintw(i + 3, 20, "|l. RING2: %37s|", d->PC->equipment[i]->get_name());
        break;
      }
    }
    else
      switch (i)
      {
      case 0:
        mvprintw(i + 3, 20, "|a. WEAPON: Empty                                |");
        break;
      case 1:
        mvprintw(i + 3, 20, "|b. OFFHAND: Empty                                |");
        break;
      case 2:
        mvprintw(i + 3, 20, "|c. RANGED: Empty                                |");
        break;
      case 3:
        mvprintw(i + 3, 20, "|d. ARMOR: Empty                                |");
        break;
      case 4:
        mvprintw(i + 3, 20, "|e. HELMET: Empty                                |");
        break;
      case 5:
        mvprintw(i + 3, 20, "|f. CLOAK: Empty                                |");
        break;
      case 6:
        mvprintw(i + 3, 20, "|g. GLOVES: Empty                                |");
        break;
      case 7:
        mvprintw(i + 3, 20, "|h. BOOTS: Empty                                |");
        break;
      case 8:
        mvprintw(i + 3, 20, "|i. AMULET: Empty                                |");
        break;
      case 9:
        mvprintw(i + 3, 20, "|j. LIGHT: Empty                                |");
        break;
      case 10:
        mvprintw(i + 3, 20, "|k. RING1: Empty                                |");
        break;
      case 11:
        mvprintw(i + 3, 20, "|l. RING2: Empty                                |");
        break;
      }
  }

  mvprintw(16, 20, "|-----------ESC to abort------------|");

  refresh();

  do
  {
    if ((key = getch()) == 27)
    {
      io_display(d);
      return;
    }
  } while (1);
}
void io_wear(dungeon *d)
{
  int key, i;
  attron(COLOR_PAIR(COLOR_CYAN));
  mvprintw(2, 20, "|%-*s|", MODE_WIDTH, "Wear Mode");
  attroff(COLOR_PAIR(COLOR_CYAN));

  for (i = 0; i < INVENTORY_SIZE; i++)
  {
    if (d->PC->inventory[i])
      mvprintw(i + 3, 20, "|%2d: %-*s|", i, MODE_WIDTH - 4, d->PC->inventory[i]->get_name());
    else
      mvprintw(i + 3, 20, "|%-*s|", MODE_WIDTH - 4, "EMPTY");
  }

  mvprintw(16, 20, "Enter esc to abort");
  refresh();

  do
  {
    if ((key = getch()) == 27)
    {
      io_display(d);
      return;
    }
    else if (key != '0' && key != '1' && key != '2' && key != '3' && key != '4' && key != '5' && key != '6' && key != '7' && key != '8' && key != '9')
    {
      mvprintw(18, 1, "Wrong number!");
      continue;
    }
    else if (!(d->PC->inventory[key - '0']))
    {
      mvprintw(18, 1, "No item!");
      refresh();
      continue;
    }
    else
    {
      d->PC->wear((key - '0'));
      io_display(d);
      return;
    }
  } while (1);
}

void io_take_off(dungeon *d)
{
  int key, i;
  attron(COLOR_PAIR(COLOR_CYAN));
  mvprintw(2, 20, "|-------------Equipment-------------|");
  attroff(COLOR_PAIR(COLOR_CYAN));

  for (i = 0; i < EQUIPMENT_SIZE; i++)
  {
    if (d->PC->equipment[i])
    {
      switch (i)
      {
      case 0:
        mvprintw(i + 3, 20, "|a. WEAPON: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 1:
        mvprintw(i + 3, 20, "|b. OFFHAND: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 2:
        mvprintw(i + 3, 20, "|c. RANGED: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 3:
        mvprintw(i + 3, 20, "|d. ARMOR: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 4:
        mvprintw(i + 3, 20, "|e. HELMET: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 5:
        mvprintw(i + 3, 20, "|f. CLOAK: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 6:
        mvprintw(i + 3, 20, "|g. GLOVES: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 7:
        mvprintw(i + 3, 20, "|h. BOOTS: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 8:
        mvprintw(i + 3, 20, "|i. AMULET: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 9:
        mvprintw(i + 3, 20, "|j. LIGHT: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 10:
        mvprintw(i + 3, 20, "|k. RING1: %37s|", d->PC->equipment[i]->get_name());
        break;
      case 11:
        mvprintw(i + 3, 20, "|l. RING2: %37s|", d->PC->equipment[i]->get_name());
        break;
      }
    }
    else
      switch (i)
      {
      case 0:
        mvprintw(i + 3, 20, "|a. WEAPON: Empty                                |");
        break;
      case 1:
        mvprintw(i + 3, 20, "|b. OFFHAND: Empty                                |");
        break;
      case 2:
        mvprintw(i + 3, 20, "|c. RANGED: Empty                                |");
        break;
      case 3:
        mvprintw(i + 3, 20, "|d. ARMOR: Empty                                |");
        break;
      case 4:
        mvprintw(i + 3, 20, "|e. HELMET: Empty                                |");
        break;
      case 5:
        mvprintw(i + 3, 20, "|f. CLOAK: Empty                                |");
        break;
      case 6:
        mvprintw(i + 3, 20, "|g. GLOVES: Empty                                |");
        break;
      case 7:
        mvprintw(i + 3, 20, "|h. BOOTS: Empty                                |");
        break;
      case 8:
        mvprintw(i + 3, 20, "|i. AMULET: Empty                                |");
        break;
      case 9:
        mvprintw(i + 3, 20, "|j. LIGHT: Empty                                |");
        break;
      case 10:
        mvprintw(i + 3, 20, "|k. RING1: Empty                                |");
        break;
      case 11:
        mvprintw(i + 3, 20, "|l. RING2: Empty                                |");
        break;
      }
  }

  mvprintw(16, 20, "|-----------ESC to abort------------|");

  refresh();

  do
  {
    if ((key = getch()) == 27)
    {
      io_display(d);
      return;
    }
    else if (key != 'a' && key != 'b' && key != 'c' && key != 'd' && key != 'e' && key != 'f' && key != 'g' && key != 'h' && key != 'i' && key != 'j' && key != 'k' && key != 'l')
    {
      mvprintw(18, 1, "Wrong number!");
      continue;
    }
    else if (!(d->PC->equipment[key - 'a']))
    {
      mvprintw(18, 1, "No item!");
      refresh();
      continue;
    }
    else
    {
      d->PC->take_off(d->PC->equipment[key - 'a']);
      io_display(d);
      return;
    }
  } while (1);
}

void io_inspect(dungeon *d)
{
  int key, i;
  attron(COLOR_PAIR(COLOR_CYAN));
  mvprintw(2, 20, "|-------------inspect-------------|");
  attroff(COLOR_PAIR(COLOR_CYAN));
  for (i = 0; i < INVENTORY_SIZE; i++)
  {
    if (d->PC->inventory[i])
      mvprintw(i + 3, 20, "|%2d: %-*s|", i, MODE_WIDTH - 4, d->PC->inventory[i]->get_name());
    else
      mvprintw(i + 3, 20, "|%-*s|", MODE_WIDTH - 4, "EMPTY");
  }

  mvprintw(16, 20, "|-----------ESC to abort------------|");

  refresh();

  do
  {
    if ((key = getch()) == 27)
    {
      io_display(d);
      return;
    }
    else if (key != '0' && key != '1' && key != '2' && key != '3' && key != '4' && key != '5' && key != '6' && key != '7' && key != '8' && key != '9')
    {
      mvprintw(18, 1, "Wrong number!");
      continue;
    }
    else if (!(d->PC->inventory[key - '0']))
    {
      mvprintw(18, 1, "No item!");
      refresh();
      continue;
    }
    else
    {
      std::string desc = d->PC->inventory[key - '0']->get_description();
      const int max_length = 78;
      size_t pos = 0;
      while (pos < desc.length())
      {
        size_t len = std::min<size_t>(max_length, desc.length() - pos);
        if (pos + len < desc.length())
        {
          while (len > 0 && desc[pos + len] != ' ')
            len--;
          if (len == 0)
            len = max_length;
        }
        io_queue_message("%s", desc.substr(pos, len).c_str());
        pos += len;
        while (pos < desc.length() && desc[pos] == ' ')
          pos++;
      }
      io_display(d);
      return;
    }
  } while (1);
}

void io_expunge(dungeon *d)
{
  int key, i;
  attron(COLOR_PAIR(COLOR_CYAN));
  mvprintw(2, 20, "|-------------expunge-------------|");
  attroff(COLOR_PAIR(COLOR_CYAN));
  for (i = 0; i < INVENTORY_SIZE; i++)
  {
    if (d->PC->inventory[i])
      mvprintw(i + 3, 20, "|%2d: %-*s|", i, MODE_WIDTH - 4, d->PC->inventory[i]->get_name());
    else
      mvprintw(i + 3, 20, "|%-*s|", MODE_WIDTH - 4, "EMPTY");
  }

  mvprintw(16, 20, "|-----------ESC to abort------------|");

  refresh();

  do
  {
    if ((key = getch()) == 27)
    {
      io_display(d);
      return;
    }
    else if (key != '0' && key != '1' && key != '2' && key != '3' && key != '4' && key != '5' && key != '6' && key != '7' && key != '8' && key != '9')
    {
      mvprintw(18, 1, "Wrong number!");
      continue;
    }
    else if (d->PC->expunge(d->PC->inventory[key - '0']))
    {
      io_display(d);
      return;
    }
    else
    {
      d->PC->inventory[key - '0'] = NULL;
      io_display(d);
      return;
    }
  } while (1);
}
void io_look(dungeon *d)
{
  pair_t dest;
  int c;
  fd_set readfs;
  struct timeval tv;

  pc_reset_visibility(d->PC);
  io_display_no_fog(d);

  mvprintw(0, 0,
           "Hover over a monster to inspect. 't' to inspect; 'esc' to abort.");

  dest[dim_y] = d->PC->position[dim_y];
  dest[dim_x] = d->PC->position[dim_x];

  mvaddch(dest[dim_y] + 1, dest[dim_x], '*');
  refresh();

  do
  {
    do
    {
      FD_ZERO(&readfs);
      FD_SET(STDIN_FILENO, &readfs);

      tv.tv_sec = 0;
      tv.tv_usec = 125000; /* An eigth of a second */

      io_redisplay_non_terrain(d, dest);
    } while (!select(STDIN_FILENO + 1, &readfs, NULL, NULL, &tv));
    /* Can simply draw the terrain when we move the cursor away, *
     * because if it is a character or object, the refresh       *
     * function will fix it for us.                              */
    switch (mappair(dest))
    {
    case ter_wall:
    case ter_wall_immutable:
    case ter_unknown:
      mvaddch(dest[dim_y] + 1, dest[dim_x], ' ');
      break;
    case ter_floor:
    case ter_floor_room:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '.');
      break;
    case ter_floor_hall:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '#');
      break;
    case ter_debug:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '*');
      break;
    case ter_stairs_up:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '<');
      break;
    case ter_stairs_down:
      mvaddch(dest[dim_y] + 1, dest[dim_x], '>');
      break;
    default:
      /* Use zero as an error symbol, since it stands out somewhat, and it's *
       * not otherwise used.                                                 */
      mvaddch(dest[dim_y] + 1, dest[dim_x], '0');
    }
    switch ((c = getch()))
    {
    case '7':
    case 'y':
    case KEY_HOME:
      if (dest[dim_y] != 1)
      {
        dest[dim_y]--;
      }
      if (dest[dim_x] != 1)
      {
        dest[dim_x]--;
      }
      break;
    case '8':
    case 'k':
    case KEY_UP:
      if (dest[dim_y] != 1)
      {
        dest[dim_y]--;
      }
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (dest[dim_y] != 1)
      {
        dest[dim_y]--;
      }
      if (dest[dim_x] != DUNGEON_X - 2)
      {
        dest[dim_x]++;
      }
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      if (dest[dim_x] != DUNGEON_X - 2)
      {
        dest[dim_x]++;
      }
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      if (dest[dim_y] != DUNGEON_Y - 2)
      {
        dest[dim_y]++;
      }
      if (dest[dim_x] != DUNGEON_X - 2)
      {
        dest[dim_x]++;
      }
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      if (dest[dim_y] != DUNGEON_Y - 2)
      {
        dest[dim_y]++;
      }
      break;
    case '1':
    case 'b':
    case KEY_END:
      if (dest[dim_y] != DUNGEON_Y - 2)
      {
        dest[dim_y]++;
      }
      if (dest[dim_x] != 1)
      {
        dest[dim_x]--;
      }
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      if (dest[dim_x] != 1)
      {
        dest[dim_x]--;
      }
      break;
    }
  } while (c != 't' && c != 27);

  if (charpair(dest) && charpair(dest) != d->PC)
  {
    io_queue_message("You saw a %s.", character_get_name(charpair(dest)));
    // dynamic case
    npc *npc_char = dynamic_cast<npc *>(charpair(dest));
    if (npc_char)
    {
      clear();
      mvprintw(2, 9, "NAME %s.", character_get_name(npc_char));
      mvprintw(4, 9, "HP: %d", npc_char->hp);
      mvprintw(5, 9, "~DAMAGE: %d", npc_char->damage->roll());
      mvprintw(6, 9, "DESC: \n\t%s", npc_char->description);
      refresh(); // Make sure the screen is updated
      attron(A_BOLD);
      mvprintw(23, 9, "Hit any key to continue.");
      attroff(A_BOLD);
      getch();
    }
  }
  else if (objpair(dest))
  {
    io_queue_message("You see the object %s.", objpair(dest)->get_name());
  }
  else if (mappair(dest) == ter_stairs_up)
  {
    io_queue_message("You see a staircase up.");
  }
  else if (mappair(dest) == ter_stairs_down)
  {
    io_queue_message("You see a staircase down.");
  }
  else if (mappair(dest) == ter_debug)
  {
    io_queue_message("You see a debug tile.");
  }
  io_display(d);
}

void io_handle_input(dungeon *d)
{
  uint32_t fail_code;
  int key;
  fd_set readfs;
  struct timeval tv;
  uint32_t fog_off = 0;
  pair_t tmp = {DUNGEON_X, DUNGEON_Y};

  do
  {
    do
    {
      FD_ZERO(&readfs);
      FD_SET(STDIN_FILENO, &readfs);

      tv.tv_sec = 0;
      tv.tv_usec = 125000; /* An eigth of a second */

      if (fog_off)
      {
        /* Out-of-bounds cursor will not be rendered. */
        io_redisplay_non_terrain(d, tmp);
      }
      else
      {
        io_redisplay_visible_monsters(d);
      }
    } while (!select(STDIN_FILENO + 1, &readfs, NULL, NULL, &tv));
    fog_off = 0;
    switch (key = getch())
    {
    case '7':
    case 'y':
    case KEY_HOME:
      fail_code = move_pc(d, 7);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      fail_code = move_pc(d, 8);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      fail_code = move_pc(d, 9);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      fail_code = move_pc(d, 6);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      fail_code = move_pc(d, 3);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      fail_code = move_pc(d, 2);
      break;
    case '1':
    case 'b':
    case KEY_END:
      fail_code = move_pc(d, 1);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      fail_code = move_pc(d, 4);
      break;
    case '5':
    case ' ':
    case '.':
    case KEY_B2:
      fail_code = 0;
      break;
    case '>':
      fail_code = move_pc(d, '>');
      break;
    case '<':
      fail_code = move_pc(d, '<');
      break;
    case 'Q':
      d->quit = 1;
      fail_code = 0;
      break;
    case 'T':
      /* New command.  Display the distances for tunnelers.             */
      io_display_tunnel(d);
      fail_code = 1;
      break;
    case 'D':
      /* New command.  Display the distances for non-tunnelers.         */
      io_display_distance(d);
      fail_code = 1;
      break;
    case 'H':
      /* New command.  Display the hardnesses.                          */
      io_display_hardness(d);
      fail_code = 1;
      break;
    case 's':
      /* New command.  Return to normal display after displaying some   *
       * special screen.                                                */
      io_display(d);
      fail_code = 1;
      break;

    case 'g':
      /* Teleport the PC to a random place in the dungeon.              */
      io_teleport_pc(d);
      fail_code = 1;
      break;
    case 'f':
      io_display_no_fog(d);
      fail_code = 1;
      break;
    case 'm':
      io_list_monsters(d);
      fail_code = 1;
      break;
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matterrs, but using this command will  *
       * waste a turn.  Set fail_code to 1 and you should be able to *
       * figure out why I did it that way.                           */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      fail_code = 0;
      break;
    case 'w':
      // wear
      io_wear(d);
      fail_code = 1;
      break;
    case 't':
      // take off
      io_take_off(d);
      break;
    case 'd':
      // drop
      io_drop(d);
      fail_code = 1;
      break;
    case 'x':
      // expunge
      io_expunge(d);
      fail_code = 1;
      break;
    case 'i':
      // inventory
      io_inventory(d);
      fail_code = 1;
      break;
    case 'e':
      // display equipment
      io_equipment(d);
      fail_code = 1;
      break;
    case 'I':
      // inspect item
      io_inspect(d);
      fail_code = 1;
      break;
    case 'L':
      // look
      io_look(d);
      fail_code = 1;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      fail_code = 1;
    }
  } while (fail_code);
}