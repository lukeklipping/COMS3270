#ifndef CHARACTER_H
#define CHARACTER_H

typedef struct pair_t pair_t;

typedef struct
{
    char symbol;
    int speed;
    // alive = 1
    int alive;
    int sequence;
    pair_t position;
    monster_t mon_character;
} character_t;

int character_compare(void *ch1, void *ch2);
void character_delete(void *c);

#endif