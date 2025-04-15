#include <cstring>

#include "object.h"
#include "dungeon.h"
#include "utils.h"

object::object(const object_description &o, pair_t p, object *next) : name(o.get_name()),
                                                                      description(o.get_description()),
                                                                      type(o.get_type()),
                                                                      color(o.get_color()),
                                                                      damage(o.get_damage()),
                                                                      hit(o.get_hit().roll()),
                                                                      dodge(o.get_dodge().roll()),
                                                                      defense(o.get_defence().roll()),
                                                                      weight(o.get_weight().roll()),
                                                                      speed(o.get_speed().roll()),
                                                                      attribute(o.get_attribute().roll()),
                                                                      value(o.get_value().roll()),
                                                                      seen(false),
                                                                      next(next)
{
    position[dim_x] = p[dim_x];
    position[dim_y] = p[dim_y];
}
object::~object()
{
    if (next)
    {
        delete next;
    }
}

void object_gen(dungeon_t *d)
{
    object *o;
    uint32_t room;
    pair_t p;
    const std::vector<object_description> &v = d->object_descriptions;
    const object_description &od = v[rand_range(0, v.size() - 1)];

    room = rand_range(0, d->num_rooms - 1);
    p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                          (d->rooms[room].position[dim_y] +
                           d->rooms[room].size[dim_y] - 1));
    p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                          (d->rooms[room].position[dim_x] +
                           d->rooms[room].size[dim_x] - 1));

    o = new object(od, p, d->object_map[p[dim_y]][p[dim_x]]);

    d->object_map[p[dim_y]][p[dim_x]] = o;
}
void gen_objects(dungeon_t *d)
{
    uint32_t i;

    memset(d->object_map, 0, sizeof(d->object_map));

    for (i = 0; i < d->max_objects; i++)
    {
        object_gen(d);
    }
    d->num_objects = d->max_objects;
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
char object::get_symbol()
{
    return next ? '&' : object_symbol[type];
}
uint32_t object::get_color()
{
    return color;
}
