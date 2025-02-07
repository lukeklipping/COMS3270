#ifndef DUNGEONGENERATION_H
#define DUNGEONGENERATION_H

#define HEIGHT 21
#define WIDTH 80

#define PLAYER '@'
#define ROCK ' '
#define ROOM '.'
#define HALL '#'

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

typedef struct dungeon{
    int num_rooms;
    Room *rooms;
}dungeon_t;

void dungeon_init();
void dungeon_print();
int is_valid(int, int, int, int);
void generate_rooms_character(dungeon_t *d);
void generate_corridor(dungeon_t *d);
void generate_stairs(dungeon_t *d);



#endif