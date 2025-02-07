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
void generate_rooms_character(dungeon_t *d){
    int num_rooms = MIN_ROOMS + rand() % (MAX_ROOMS - MIN_ROOMS + 1);
    int room_count = 0;
    int j, i;

    //dungeon_t *d = malloc(sizeof(dungeon_t));
    d->rooms = malloc(num_rooms * sizeof(Room));
    d->num_rooms = 0;

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
            d->rooms[room_count].x = x;
            d->rooms[room_count].y = y;
            d->rooms[room_count].width = width;
            d->rooms[room_count].height = height;
            /*   
            (*r)[room_count].x = x;
            (*r)[room_count].y = y;
            (*r)[room_count].width = width;
            (*r)[room_count].height = height;*/
            d->num_rooms++;
            room_count++;
        }
    }   
    int px;
    int py;
    do{
        px = d->rooms->x +rand() % d->rooms[0].width;
        py = d->rooms->y + rand() % d->rooms[0].height;  
    } while(dungeon[py][px] != ROOM);
    
    dungeon[py][px] = PLAYER;
    
}


// generates hallways from room to room, continues past room characters
void generate_corridor(dungeon_t *d){
    int i;
    for(i = 0; i < d->num_rooms -1; i++){
        //printf("%d %d %d %d", (*r)[i].x, (*r)[i].y, (*r)[i].width, (*r)[i].height);
        int x1 = d->rooms[i].x + d->rooms[i].width /2;
        int y1 = d->rooms[i].y + d->rooms[i].height /2;
        int x2 = d->rooms[i + 1].x + d->rooms[i + 1].width /2;
        int y2 = d->rooms[i + 1].y + d->rooms[i + 1].height /2;

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

void generate_stairs(dungeon_t *d){
    char stairs[2] = {'<', '>'};
    int i, y, x;
    for(i = 0; i < 2; i++){
        int room_index = rand() % d->num_rooms;
        do{
            y = d->rooms[room_index].y +  (rand() % d->rooms[room_index].height);
            x = d->rooms[room_index].x + (rand() % d->rooms[room_index].width);
        } while(dungeon[y][x] != ROOM);
        dungeon[y][x] = stairs[i];
        hardness[y][x] = 0;
    }
}

void save_dungeon(dungeon_t *d){
    char *path;
    FILE *f = fopen(path, "w");
    uint8_t write_to8;
    uint16_t write_to16;
    uint32_t write_to32;

    char *semantic = "RLG327-S2025";
    //remove null
    fwrite(semantic, 1, strlen(semantic), f);
    


    int version = 0;
    write_to32 = htobe32(version);
    fwrite(&write_to32, sizeof(write_to32), 1, f);

    //integer size file
    
    //A pair of unsigned 8-bit integers giving the x and y position of the PC
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(dungeon[i][j] == PLAYER){
                //y
                write_to8 = (uint8_t)j;
                fwrite(&write_to8, sizeof(write_to8), 1, f);

                //x
                write_to8 = (uint8_t)i;
                fwrite(&write_to8, sizeof(write_to8), 1, f);
                break;
            }
        }
    }

    
    //The row-major dungeon matrix from top to bottom, with one byte, containing cell hardness, per cell. The hardness ranges from zero to 255, with
    //zero representing open space (room or corridor) and 255 representing immutable rock (probably only the border).
    

   //r, an unsigned 16-bit integer giving the number of rooms in the dungeon (2 bytes)
   write_to16 = htobe16((uint16_t)d->num_rooms);
   fwrite(&write_to16, sizeof(write_to16), 1, f);
    
}





int main(int argc, char *argv[]){
    dungeon_t dungeon;
    srand(time(NULL));

    dungeon_init();
    generate_rooms_character(&dungeon);
    
    // optional to place next two functions within generate_rooms
    generate_corridor(&dungeon);
    generate_stairs(&dungeon);
    // ^^
    dungeon_print();

    // frees rooms memory
    free(dungeon.rooms);
    return 0;
}

