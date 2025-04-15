#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include "descriptions.h"
#include "dims.h"

typedef struct dungeon dungeon_t;

class object
{
public:
    const std::string &name;
    const std::string &description;
    object_type_t type;
    char symbol;
    uint32_t color;
    pair_t position;
    const dice damage;
    int32_t hit, dodge, defense, weight, speed, attribute, value;
    bool seen;
    object *next;
    object(const object_description &o, pair_t p, object *next);
    ~object();
    char get_symbol();
    uint32_t get_color();
    bool have_seen() { return seen; }
    void has_been_seen() { seen = true; }
};
extern const char object_symbol[];

// void object_gen(dungeon_t *d);
void objects_delete(dungeon_t *d);
void gen_objects(dungeon_t *d);
#endif