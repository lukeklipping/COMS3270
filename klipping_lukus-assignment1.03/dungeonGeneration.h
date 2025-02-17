#ifndef DUNGEONGENERATION_H
#define DUNGEONGENERATION_H
#include<stdint.h>

#include "heap.h"


#define DUNGEON_VERSIONX 0
#define DUNGEON_SAVE_FILE "dungeon"
#define SAVE_DIR ".rlg327"

#define HEIGHT 21
#define WIDTH 80

#define PLAYER '@'
#define ROCK ' '
#define ROOM '.'
#define HALL '#'
#define IMMUTABLE_WALL ' '

#define MIN_WIDTH 4
#define MIN_HEIGHT 3

//arbitrary value to set upper limit
#define MAX_ROOMS 8
#define MIN_ROOMS 6

typedef struct Room{
    int x, y, height, width;
}Room;

//actions for flags
typedef enum{
    action_save,
    action_load,
    action_read,
    action_rand
} action_t;

typedef struct {
    uint8_t y;
    uint8_t x;
} pair_t;

typedef struct dungeon{
    char map[HEIGHT][WIDTH];
    int num_rooms;
    Room *rooms;
    uint8_t hardness[HEIGHT][WIDTH];
    int PC_N[HEIGHT][WIDTH];
    int PC_T[HEIGHT][WIDTH];
    pair_t PC;
    //pair_t *stairs;
}dungeon_t;


void dungeon_init(dungeon_t *d);
void dungeon_print(dungeon_t *d);
int is_valid(dungeon_t *d, int, int, int, int);
void generate_rooms_character(dungeon_t *d);
void generate_corridor(dungeon_t *d);
void generate_stairs(dungeon_t *d);
int generate_PC(dungeon_t *d);
int count_up_stairs(dungeon_t *d);
int count_down_stairs(dungeon_t *d);
int read_dungeon_map(dungeon_t *d, FILE* f);
int read_rooms(dungeon_t *d, FILE *f);
int read_stairs(dungeon_t *d, FILE *f);
int load_dungeon(dungeon_t *d, char *f);
int save_dungeon(dungeon_t *d, char *f);
int save_stairs(dungeon_t *d, FILE *f);


#endif