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
    int32_t hit, damage, dodge, defense, weight, speed, attribute, value;
    /*object(const object_description &od) : name(od.get_name()),
                                           description(od.get_description()),
                                           type(od.get_type()),
                                           color(od.get_color()),
                                           position(),
                                           damage(od.get_damage()),
                                           hit(od.get_hit()),
                                           damage(od.get_damage()),
                                           dodge(od.get_dodge()),
                                           defense(od.get_defence()),
                                           weight(od.get_weight()),
                                           speed(od.get_speed()),
                                           attribute(od.get_attribute()),
                                           value(od.get_value()) {};*/
};

void object_gen(dungeon_t *d);
void objects_delete(dungeon_t *d);
#endif