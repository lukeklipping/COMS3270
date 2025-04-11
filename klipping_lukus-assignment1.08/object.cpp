#include <cstring>

#include "object.h"
#include "dungeon.h"
#include "utils.h"
extern const char object_symbol[] = {
    '*',  /* objtype_no_type */
    '|',  /* objtype_WEAPON */
    ')',  /* objtype_OFFHAND */
    '}',  /* objtype_RANGED */
    '~',  /* objtype_LIGHT */
    '[',  /* objtype_ARMOR */
    ']',  /* objtype_HELMET */
    '(',  /* objtype_CLOAK */
    '{',  /* objtype_GLOVES */
    '\\', /* objtype_BOOTS */
    '"',  /* objtype_AMULET */
    '=',  /* objtype_RING */
    '`',  /* objtype_SCROLL */
    '?',  /* objtype_BOOK */
    '!',  /* objtype_FLASK */
    '$',  /* objtype_GOLD */
    '/',  /* objtype_AMMUNITION */
    ',',  /* objtype_FOOD */
    '-',  /* objtype_WAND */
    '%',  /* objtype_CONTAINER */
};

object::object(dungeon_t *d, const object_description &o)
{
    uint32_t room;
    pair_t p;

    symbol = object_symbol[o.type];
    color = o.color;
    name = o.name.c_str();
    description = o.description.c_str();

    // Randomly place object in a room (not the PC room, usually room 0)
    do
    {
        room = rand_range(1, d->num_rooms - 1);
        p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                              d->rooms[room].position[dim_y] + d->rooms[room].size[dim_y] - 1);
        p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                              d->rooms[room].position[dim_x] + d->rooms[room].size[dim_x] - 1);
    } while (d->object_map[p[dim_y]][p[dim_x]]); // Ensure object cell is free

    position[dim_y] = p[dim_y];
    position[dim_x] = p[dim_x];

    d->object_map[p[dim_y]][p[dim_x]] = this;

    // Any other attributes from object_description you'd like to store
    // like damage, speed, weight, etc., depending on your object model
}

void object_gen(dungeon_t *d)
{

    /*
    uint32_t i;
    d->num_objects = 15;
    object *o;
    uint32_t room;
    pair_t position;

    std::vector<object_description> &ov = d->object_descriptions;
    object_description &od = ov[rand_range(0, ov.size() - 1)];
    o = od.generate_object(d);

    room = rand_range(1, d->num_rooms - 1);
    position[dim_y] = rand_range(d->rooms[room].position[dim_y],
                                 (d->rooms[room].position[dim_y] +
                                  d->rooms[room].size[dim_y] - 1));
    position[dim_x] = rand_range(d->rooms[room].position[dim_x],
                                 (d->rooms[room].position[dim_x] +
                                  d->rooms[room].size[dim_x] - 1));

    d->object_map[position[dim_y]][position[dim_x]] = o;*/
}
void gen_objects(dungeon_t *d)
{
    uint32_t i;
    memset(d->object_map, 0, sizeof(d->object_map));

    for (i = 0; i < d->num_objects; i++)
    {
        object_description::generate_object(d);
    }
}

void objects_delete(dungeon_t *d)
{
    uint32_t x, y;
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {
            if (d->object_map[y][x])
            {
                delete d->object_map[y][x];
                d->object_map[y][x] = nullptr;
            }
        }
    }
    d->num_objects = 0;
}