#ifndef DUNGEONGENERATION_H
#define DUNGEONGENERATION_H

#define HEIGHT 21
#define WIDTH 80

#define ROCK ' '
#define ROOM '.'
#define HALL '#'

#define MIN_WIDTH 4
#define MIN_HEIGHT 3

//arbitrary value to set upper limit
#define MAX_ROOMS 9
#define MIN_ROOMS 6

typedef struct Room{
    int x, y, height, width;
}Room;

void dungeon_init();
void dungeon_print();
int is_valid(int, int, int, int);
int generate_rooms(Room **r);
void generate_corridor(Room **r, int);
void generate_stairs(Room **r, int);



#endif