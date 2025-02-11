#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<endian.h>
#include<string.h>

#include "dungeonGeneration.h"


char dungeon[HEIGHT][WIDTH];
int hardness[HEIGHT][WIDTH];




// fills dungeon with rock and initializes hardness
void dungeon_init(dungeon_t *d){
    
    int x, y;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT  - 1){
                dungeon[y][x] = ROCK;
                d->hardness[y][x] = IMMUTABLE_WALL;
            }else{
                dungeon[y][x] = ROCK;
                d->hardness[y][x] = 1 + rand() % 254;
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
    int j, i, px, py;

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

            d->num_rooms++;
            room_count++;
        }
    }   

    do{
        px = d->rooms->x +rand() % d->rooms[0].width;
        py = d->rooms->y + rand() % d->rooms[0].height;  
    } while(dungeon[py][px] != ROOM);

    d->PC.x = px;
    d->PC.y = py;
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
                    d->hardness[y1][x] = 0;
                }
                

            }
        } else {
            for (int x = x1; x >= x2; x--) {
                if (dungeon[y1][x] != ROOM){
                    dungeon[y1][x] = HALL;
                    d->hardness[y1][x] = 0;
                }
                

            }
        }

        // vertical corridor 
        if (y1 < y2) {
            for (int y = y1; y <= y2; y++) {
                if (dungeon[y][x2] != ROOM){
                    dungeon[y][x2] = HALL;
                    d->hardness[y][x2] = 0;
                }
                
            }
        } else {
            for (int y = y1; y >= y2; y--) {
                if (dungeon[y][x2] != ROOM){
                    dungeon[y][x2] = HALL;
                    d->hardness[y][x2] = 0;
                }
                
            }
        }
    }
}
// generates both stairs
void generate_stairs(dungeon_t *d){
    //< up
    //> down
    char stairs[2] = {'<', '>'};
    int i, y, x;
    //d->stairs = malloc(2 * sizeof(pair_t));

    for(i = 0; i < 2; i++){
        int room_index = rand() % d->num_rooms;
        do{
            y = d->rooms[room_index].y +  (rand() % d->rooms[room_index].height);
            x = d->rooms[room_index].x + (rand() % d->rooms[room_index].width);
        } while(dungeon[y][x] != ROOM);
        //d->stairs[i].x = x;
        //d->stairs[i].y = y;
        dungeon[y][x] = stairs[i];
        d->hardness[y][x] = 0;
    }
   // d->num_up_stairs++;
    //d->num_down_stairs++;
}

void save_dungeon(dungeon_t *d){
    char *path;
    FILE *f = fopen(path, "w");
    uint16_t write_to16;
    uint32_t write_to32;

    char *semantic = "RLG327-S2025";
    fwrite(semantic, 1, strlen(semantic), f);
    


    
    write_to32 = htobe32(DUNGEON_VERSIONX);
    fwrite(&write_to32, sizeof(write_to32), 1, f);

    //integer size file
    
    //A pair of unsigned 8-bit integers giving the x and y position of the PC
    uint8_t x = d->PC.x;
    uint8_t y = d->PC.y;
    fwrite(&x, 1, 1, f);
    fwrite(&y, 1, 1, f);
    
    //The row-major dungeon matrix from top to bottom, with one byte, containing cell hardness, per cell. The hardness ranges from zero to 255, with
    //zero representing open space (room or corridor) and 255 representing immutable rock (probably only the border).
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j <WIDTH; j++){
            fwrite(&d->hardness[i][j], 1,1,f);
        }
    }
    
   //r, an unsigned 16-bit integer giving the number of rooms in the dungeon (2 bytes)
   write_to16 = htobe16((uint16_t)d->num_rooms);
   fwrite(&write_to16, sizeof(write_to16), 1, f);

   //The positions of all of the rooms in the dungeon, given with 4 unsigned
    //8-bit integers each. The first byte is the x position of the upper left corner
    //of the room; the second byte is the y position of the upper left corner of the
    //room; the third byte is the x size (width) of the room; and the fourth byte
    //is the y size (height) of the room. r is the number of rooms in the dungeon.
    for(int i = 0; i < d->num_rooms; i++){
        uint8_t xpos = d->rooms[i].x;
        uint8_t ypos = d->rooms[i].y;
        uint8_t xdim = d->rooms[i].width;
        uint8_t ydim = d->rooms[i].height;

        fwrite(&xpos, sizeof(uint8_t), 1, f);
        fwrite(&ypos, sizeof(uint8_t), 1, f);
        fwrite(&xdim, sizeof(uint8_t), 1, f);
        fwrite(&ydim, sizeof(uint8_t), 1, f);
    }

    //saves both stairs with x and y values
    save_stairs(d,f);

    fclose(f);
}

int save_stairs(dungeon_t *d, FILE *f){
    uint16_t numStairs;
    numStairs = htobe16(count_up_stairs(d));
    fwrite(&numStairs, 2, 1, f);
    int x,y;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(dungeon[y][x] == '<'){
                numStairs--;
                fwrite(&x, 1, 1, f);
                fwrite(&y, 1, 1, f);
            }
        }
    }
    numStairs = htobe16(count_down_stairs(d));
    fwrite(&numStairs, 2, 1, f);
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(dungeon[y][x] == '<'){
                numStairs--;
                fwrite(&x, 1, 1, f);
                fwrite(&y, 1, 1, f);
            }
        }
    }
    return 0;
}

// helper method to count '<'
int count_up_stairs(dungeon_t *d){
    int x, y, i;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(dungeon[y][x] == '<'){
                i++;
            }
        }
    }
    return i;
}

// helper method to count '>'
int count_down_stairs(dungeon_t *d){
    int x, y, i;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(dungeon[y][x] == '>'){
                i++;
            }
        }
    }
    return i;
}

// fileread helper to place dungeon map
int read_dungeon_map(dungeon_t *d, FILE* f){
    int y, x;

    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x< WIDTH; x++){
            fread(&d->hardness[y][x], 1, 1, f);
            if(&d->hardness[y][x] == 0){
                dungeon[y][x] = HALL;
            } else if(&d->hardness[y][x] == 255){
                dungeon[y][x] = IMMUTABLE_WALL;
            } else{
                dungeon[y][x] == ROCK;
                //could factor in immutable
            }
        }
    }
    return 0;
}

// reads rooms from file and places them into dungeon array
int read_rooms(dungeon_t *d, FILE *f){
    int i, y, x;
    uint16_t r;

    fread(&r, 2, 1, f);
    d->num_rooms = be16toh(r);
    d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

    for(i = 0; i < d->num_rooms; i++){
        fread(&r, 1, 1, f);
        d->rooms[i].x = r;
        fread(&r, 1, 1, f);
        d->rooms[i].y = r;
        fread(&r, 1, 1, f);
        d->rooms[i].width = r;
        fread(&r, 1, 1, f);
        d->rooms[i].height = r;


        for(y = d->rooms[i].y; y < d->rooms[i].y + d->rooms[i].height; y++){
            for(x = d->rooms[i].x; x < d->rooms[i].x + d->rooms[i].width; x++){
                dungeon[y][x] = ROOM;
            }
        }
    }
    return 0;
}

//helper method to read stairs and place into dungeon
int read_stairs(dungeon_t *d, FILE *f){
    uint16_t stairs;
    uint8_t x, y;

    fread(&stairs, 2, 1, f);
    stairs = be16toh(stairs);
    
    //up
    while(stairs--){
        fread(&x, 1, 1, f);
        fread(&y, 1, 1, f);
        dungeon[y][x] = '<';
    }
    //down
    fread(&stairs, 2, 1, f);
    stairs = be16toh(stairs);
    while(stairs--){
        fread(&x, 1, 1, f);
        fread(&y, 1, 1, f);
        dungeon[y][x] = '>';
    }
    return 0;
}

void load_dungeon(dungeon_t *d){
    char semantic[sizeof("RLG327-S2025")];
    FILE *f;
    uint32_t save_to32;

    fread(semantic, sizeof("RLG327-S2025") - 1, 1, f);
    fread(&save_to32, sizeof(save_to32), 1, f);
    fread(&save_to32, sizeof(save_to32), 1, f);

    fread(&d->PC.x, 1, 1, f);
    fread(&d->PC.y, 1, 1, f);

    read_dungeon_map(d, f);
    
    read_rooms(d, f);

    read_stairs(d, f);

    fclose(f);
}





int main(int argc, char *argv[]){
    dungeon_t dungeon;
    srand(time(NULL));

    dungeon_init(&dungeon);
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

