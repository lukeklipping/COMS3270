#ifndef CHARACTER_H
#define CHARACTER_H

typedef struct pair_t pair_t;

typedef struct{
    char symbol;
    int speed;
    int alive;
    pair_t position;

} character_t;

int character_compare(void *ch1, void *ch2);
void character_delete(void *c);

#endif