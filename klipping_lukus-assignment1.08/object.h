#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include "dims.h"
#include "dice.h"
#include "descriptions.h"

class object
{
public:
    const std::string &name;
    const std::string &description;
    object_type_t type;
    char symbol;
    uint32_t color;
    pair_t position;
    dice damage;
    int32_t hit, dodge, defense, weight, speed, attribute, value;
    object(const object_description &obj_desc, pair_t pos, object *next)
        : name(obj_desc.get_name()),
          description(obj_desc.get_description()),
          type(obj_desc.get_type()),
          symbol(object_symbol[obj_desc.get_type()]), // Assumes object_symbol maps types to chars
          color(obj_desc.get_color()),
          damage(obj_desc.get_damage()),
          hit(obj_desc.get_hit().roll()),
          dodge(obj_desc.get_dodge().roll()),
          defense(obj_desc.get_defence().roll()),
          weight(obj_desc.get_weight().roll()),
          speed(obj_desc.get_speed().roll()),
          attribute(obj_desc.get_attribute().roll()),
          value(obj_desc.get_value().roll())
    {
        position[dim_x] = pos[dim_x];
        position[dim_y] = pos[dim_y];
    }
    ~object() {};
};

void object_gen(dungeon_t *d);
void objects_delete(dungeon_t *d);
#endif