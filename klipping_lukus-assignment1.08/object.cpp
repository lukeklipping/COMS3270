#include "object.h"
#include "dungeon.h"
#include "utils.h"

void object_gen(dungeon_t *d)
{
    uint32_t i;
    d->num_objects = 15;
    object *o;
    uint32_t room;
    pair_t position;

    std::vector<object_description> &ov = d->object_descriptions;
    object_description &od = ov[rand_range(0, ov.size() - 1)];

    for (i = 0; i < d->num_objects; i++)
    {
        o = od.gen_object();
        room = rand_range(1, d->num_rooms - 1);
        position[dim_y] = rand_range(d->rooms[room].position[dim_y],
                                     (d->rooms[room].position[dim_y] +
                                      d->rooms[room].size[dim_y] - 1));
        position[dim_x] = rand_range(d->rooms[room].position[dim_x],
                                     (d->rooms[room].position[dim_x] +
                                      d->rooms[room].size[dim_x] - 1));

        o->position[dim_x] = position[dim_x];
        o->position[dim_y] = position[dim_y];
        d->object_map[position[dim_y]][position[dim_x]] = o;
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
}