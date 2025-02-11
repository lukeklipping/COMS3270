#ifndef DUNGEONGENERATION_H
#define DUNGEONGENERATION_H
#include<stdint.h>


#define DUNGEON_VERSIONX 0

#define HEIGHT 21
#define WIDTH 80

#define PLAYER '@'
#define ROCK ' '
#define ROOM '.'
#define HALL '#'
#define IMMUTABLE_WALL 255

#define MIN_WIDTH 4
#define MIN_HEIGHT 3

//arbitrary value to set upper limit
#define MAX_ROOMS 8
#define MIN_ROOMS 6

typedef struct Room{
    int x, y, height, width;
}Room;


typedef enum{
    action_save,
    action_load
} action;

typedef struct {
    uint8_t y;
    uint8_t x;
} pair_t;

typedef struct dungeon{
    int num_rooms;
    Room *rooms;
    uint8_t hardness[HEIGHT][WIDTH];
    pair_t PC;
    //pair_t *stairs;
}dungeon_t;

void dungeon_init();
void dungeon_print();
int is_valid(int, int, int, int);
void generate_rooms_character(dungeon_t *d);
void generate_corridor(dungeon_t *d);
void generate_stairs(dungeon_t *d);


#endif