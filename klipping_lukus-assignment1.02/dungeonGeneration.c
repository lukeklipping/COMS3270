#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<endian.h>
#include<stdint.h>
#include<string.h>

#include "dungeonGeneration.h"


char dungeon[HEIGHT][WIDTH];
int hardness[HEIGHT][WIDTH];


// fills dungeon with rock and initializes hardness
void dungeon_init(){
    int x, y;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT  - 1){
                dungeon[y][x] = ROCK;
                hardness[y][x] = 255;
            }else{
                dungeon[y][x] = ROCK;
                hardness[y][x] = 1 + rand() % 254;
            }
            
        }
    }
}
// prints dungeon
void dungeon_print(){
    int x, y;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            printf("%c", dungeon[y][x]);
        }
        printf("\n");
    }
}

// checks if room is valid within bounds
int is_valid(int x, int y, int w, int h){
    int i, j;
    if((x + w >= WIDTH - 1) || (y + h >= HEIGHT - 1)){
        return 0;
    }
    
    for(i = y - 2; i < y + h + 2; i++){
        for(j = x - 2; j < x + w + 2; j++){
            if(i >= 0 && j >= 0 && i < HEIGHT && j < WIDTH && dungeon[i][j] == ROOM){
                return 0;
            }
        }
    }
    return 1;
}

// generates rooms and returns room count and places character
int generate_rooms_character(Room **r){
    int num_rooms = MIN_ROOMS + rand() % (MAX_ROOMS - MIN_ROOMS + 1);
    int room_count = 0;
    int j, i;

    *r = malloc(num_rooms * sizeof(Room));

    while(room_count < num_rooms){
        int width = MIN_WIDTH + rand() % 12;
        //printf("%d", width);
        int height = MIN_HEIGHT + rand() % 8;
        //printf("%d", height);

        //keeps outermost barrier
        int x = 1 + rand() % (WIDTH - width - 2);
        int y = 1 + rand() % (HEIGHT - height - 2);

        if(is_valid(x, y, width, height)){
            for(i = y; i < y + height; i++){
                for(j = x; j < x + width; j++){
                    dungeon[i][j] = ROOM;
                    hardness[i][j] = 0;
                }
            }
            (*r)[room_count].x = x;
            (*r)[room_count].y = y;
            (*r)[room_count].width = width;
            (*r)[room_count].height = height;

            room_count++;
        }
    }   
    int px;
    int py;
    do{
        px = (*r)[0].x + rand() % (*r)[0].width;
        py = (*r)[0].y + rand() % (*r)[0].height;  
    } while(dungeon[py][px] != ROOM);
    
    dungeon[py][px] = PLAYER;
    
    return room_count;
}


// generates hallways from room to room, continues past room characters
void generate_corridor(Room **r, int num_rooms){
    int i;
    for(i = 0; i < num_rooms -1; i++){
        //printf("%d %d %d %d", (*r)[i].x, (*r)[i].y, (*r)[i].width, (*r)[i].height);
        int x1 = (*r)[i].x + (*r)[i].width /2;
        int y1 = (*r)[i].y + (*r)[i].height /2;
        int x2 = (*r)[i + 1].x + (*r)[i + 1].width /2;
        int y2 = (*r)[i + 1].y + (*r)[i + 1].height /2;

        // horizontal corridor
        if (x1 < x2) {
            for (int x = x1; x <= x2; x++) {
                if (dungeon[y1][x] != ROOM){
                    dungeon[y1][x] = HALL;
                    hardness[y1][x] = 0;
                }
                

            }
        } else {
            for (int x = x1; x >= x2; x--) {
                if (dungeon[y1][x] != ROOM){
                    dungeon[y1][x] = HALL;
                    hardness[y1][x] = 0;
                }
                

            }
        }

        // vertical corridor 
        if (y1 < y2) {
            for (int y = y1; y <= y2; y++) {
                if (dungeon[y][x2] != ROOM){
                    dungeon[y][x2] = HALL;
                    hardness[y][x2] = 0;
                }
                
            }
        } else {
            for (int y = y1; y >= y2; y--) {
                if (dungeon[y][x2] != ROOM){
                    dungeon[y][x2] = HALL;
                    hardness[y][x2] = 0;
                }
                
            }
        }
    }
}

void generate_stairs(struct Room **r, int num_rooms){
    char stairs[2] = {'<', '>'};
    int i, y, x;
    for(i = 0; i < 2; i++){
        int room_index = rand() % num_rooms;
        do{
            y = (*r)[room_index].y +  (rand() % (*r)[room_index].height);
            x = (*r)[room_index].x + (rand() % (*r)[room_index].width);
        } while(dungeon[y][x] != ROOM);
        dungeon[y][x] = stairs[i];
        hardness[y][x] = 0;
    }
}

void save_dungeon(){
    char *path;
    FILE *f = fopen(path, "w");

    char *semantic = "RLG327-S2025";

    //remove null
    fwrite(semantic, 1, strlen(semantic) -1, f);
    
    int version = 0;
    uint32_t write_to = htobe32(version);
    fwrite(&write_to, sizeof(write_to), 1, f);

}


int main(int argc, char *argv[]){
    struct Room *rooms = NULL;
    srand(time(NULL));

    dungeon_init();
    int num_rooms = generate_rooms_character(&rooms);
    
    // optional to place next two functions within generate_rooms
    generate_corridor(&rooms, num_rooms);
    generate_stairs(&rooms, num_rooms);
    // ^^
    dungeon_print();

    // frees rooms memory
    free(rooms);
    return 0;
}

