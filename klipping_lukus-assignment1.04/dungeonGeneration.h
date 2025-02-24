#ifndef DUNGEONGENERATION_H
#define DUNGEONGENERATION_H
#include<stdint.h>

#include "heap.h" 
#include "character.h"


#define DUNGEON_VERSIONX 0
#define DUNGEON_SAVE_FILE "dungeon"
#define SAVE_DIR ".rlg327"

#define DUNGEON_Y 21
#define DUNGEON_X 80

#define PLAYER '@'
#define ROCK ' '
#define ROOM '.'
#define HALL '#'
#define IMMUTABLE_WALL ' '

#define MIN_DUNGEON_X 4
#define MIN_DUNGEON_Y 3

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
    char map[DUNGEON_Y][DUNGEON_X];
    int num_rooms;
    Room *rooms;
    uint8_t hardness[DUNGEON_Y][DUNGEON_X];
    int PC_N[DUNGEON_Y][DUNGEON_X];
    int PC_T[DUNGEON_Y][DUNGEON_X];
    character_t *character[DUNGEON_Y][DUNGEON_X];
    pair_t PC;
    int num_monsters;
} dungeon_t;


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
int random_range(int, int);



#endif