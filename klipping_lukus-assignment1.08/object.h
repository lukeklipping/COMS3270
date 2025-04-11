#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include "dims.h"
#include "dice.h"
#include "descriptions.h"

class object
{
public:
    object(dungeon_t *d, const object_description &o);
    const std::string &name;
    const std::string &description;
    object_type_t type;
    char symbol;
    uint32_t color;
    pair_t position;
    dice damage;
    int32_t hit, dodge, defense, weight, speed, attribute, value;
    bool seen;
    object *next;
    // object(const object_description &o, pair_t p, object *next);
};
extern const char object_symbol[];

void object_gen(dungeon_t *d);
void objects_delete(dungeon_t *d);
void gen_objects(dungeon_t *d);
#endif