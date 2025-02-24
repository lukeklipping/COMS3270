#include<stdlib.h>

#include "character.h"
#include "monster.h"
#include "dungeonGeneration.h"
#include "heap.h"

int monsters_generate(dungeon_t *d){
    int i; 
    character_t *monster;
    int room;
    pair_t pos;

    d->num_monsters = 10;
    for(i = 0; i < d->num_monsters; i++){
        monster = malloc(sizeof(*monster));

        // find location for pos
        do{
            room = random_range(0, d->num_rooms - 1);
            pos.y = random_range(d->rooms[room].y, d->rooms[room].y + d->rooms[room].height - 1);
            pos.x = random_range(d->rooms[room].x, d->rooms[room].x + d->rooms[room].width - 1);
        } while(d->character[pos.y][pos.x]);
        // set characteristics
        monster->position.y = pos.y;
        monster->position.x = pos.x;
        d->character[pos.y][pos.x] = monster;
        monster->speed = random_range(MONSTER_MIN_SPEED, MONSTER_MAX_SPEED);
        

    }
}