#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

#include "dungeonGeneration.h"
#include "path.h"
#include "readWriteDungeon.h"


//char dungeon[DUNGEON_Y][DUNGEON_X];
// fills dungeon with rock and initializes hardness
void dungeon_init(dungeon_t *d){
    
    int x, y;
    for(y = 0; y < DUNGEON_Y; y++){
        for(x = 0; x < DUNGEON_X; x++){
            if(x == 0 || y == 0 || x == DUNGEON_X - 1 || y == DUNGEON_Y  - 1){
                d->map[y][x] = ROCK;
                d->hardness[y][x] = IMMUTABLE_WALL;
            }else{
                d->map[y][x] = ROCK;
                d->hardness[y][x] = 1 + rand() % 254;
            }
            
        }
    }
}
// prints dungeon
void dungeon_print(dungeon_t *d){
    int x, y;
    for(y = 0; y < DUNGEON_Y; y++){
        for(x = 0; x < DUNGEON_X; x++){
            printf("%c", d->map[y][x]);
        }
        printf("\n");
    }
}

// checks if room is valid within bounds
int is_valid(dungeon_t *d,int x, int y, int w, int h){
    int i, j;
    if((x + w >= DUNGEON_X - 1) || (y + h >= DUNGEON_Y - 1)){
        return 0;
    }
    
    for(i = y - 2; i < y + h + 2; i++){
        for(j = x - 2; j < x + w + 2; j++){
            if(i >= 0 && j >= 0 && i < DUNGEON_Y && j < DUNGEON_X && d->map[i][j] == ROOM){
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
        int width = MIN_DUNGEON_X + rand() % 12;
        //printf("%d", width);
        int height = MIN_DUNGEON_Y + rand() % 8;
        //printf("%d", height);

        //keeps outermost barrier
        int x = 1 + rand() % (DUNGEON_X - width - 2);
        int y = 1 + rand() % (DUNGEON_Y - height - 2);

        if(is_valid(d,x, y, width, height)){
            for(i = y; i < y + height; i++){
                for(j = x; j < x + width; j++){
                    d->map[i][j] = ROOM;
                    d->hardness[i][j] = 0;
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
}

//generates pc coords and places on map
int generate_PC(dungeon_t *d){
    int px, py;
    do{
        px = d->rooms->x +rand() % d->rooms[0].width;
        py = d->rooms->y + rand() % d->rooms[0].height;  
    } while(d->map[py][px] != ROOM || d->map[py][px] == '>' || d->map[py][px] == '<');

    d->PC.x = px;
    d->PC.y = py;
    d->map[py][px] = PLAYER;
    return 0;
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
                if (d->map[y1][x] != ROOM){
                    d->map[y1][x] = HALL;
                    d->hardness[y1][x] = 0;
                }
                

            }
        } else {
            for (int x = x1; x >= x2; x--) {
                if (d->map[y1][x] != ROOM){
                    d->map[y1][x] = HALL;
                    d->hardness[y1][x] = 0;
                }
                

            }
        }

        // vertical corridor 
        if (y1 < y2) {
            for (int y = y1; y <= y2; y++) {
                if (d->map[y][x2] != ROOM){
                    d->map[y][x2] = HALL;
                    d->hardness[y][x2] = 0;
                }
                
            }
        } else {
            for (int y = y1; y >= y2; y--) {
                if (d->map[y][x2] != ROOM){
                    d->map[y][x2] = HALL;
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
        } while(d->map[y][x] != ROOM);
        //d->stairs[i].x = x;
        //d->stairs[i].y = y;
        d->map[y][x] = stairs[i];
        d->hardness[y][x] = 0;
    }
   // d->num_up_stairs++;
    //d->num_down_stairs++;
}


// generates dungeon for --rand
void dungeon_generate(dungeon_t *d){
    generate_rooms_character(d);
    generate_corridor(d);
    generate_stairs(d);
    generate_PC(d);
}

// usage flags for command line
void usage(const char *s){
    fprintf(stderr, "%s [--load|--save|--rand]", s);
}

// frees dungeon variables
int delete_dungeon(dungeon_t *d){
    free(d->rooms);
}

// returns random number within boundaries given (helper)
int random_range(int r1, int r2){
    // check incase of user error
    // my own lol
    if(r1 > r2){
        int temp = r1;
        r1 = r2;
        r2 = temp;
    }
    return rand() % (r2 - r1 + 1) + r1;
}


// main
int main(int argc, char *argv[]){
    int i;
    action_t action; // flags for command line
    char *saveFile = NULL, *loadFile = NULL;
    dungeon_t dungeon;
    srand(time(NULL));

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--load") == 0) {
                if (i + 1 < argc) {
                    loadFile = argv[++i];
                    action = action_load;  
                } else {
                    loadFile = NULL;
                    action = action_load;
                }
            } else if (strcmp(argv[i], "--save") == 0) {
                if (i + 1 < argc) {
                    saveFile = argv[++i];
                    action = action_save;  
                } else {
                    saveFile = NULL;
                    action = action_save;  
                }
            } else if (strcmp(argv[i], "--rand") == 0) {
                action = action_rand;  
            } else {
                usage(argv[0]);  //unknown argument
                return 1;
            }
        }
    } else {
        usage(argv[0]);
        return 1;
    }

    dungeon_init(&dungeon);

    switch (action){
        case action_load:
            if(loadFile){
                load_dungeon(&dungeon, loadFile);
            } else{
                load_dungeon(&dungeon, NULL);
            }
            break;
        case action_save:
            if(saveFile){
                dungeon_generate(&dungeon);
                save_dungeon(&dungeon, saveFile);
            } else{
                dungeon_generate(&dungeon);
                save_dungeon(&dungeon, NULL);
            }
            break;
        case action_rand:
            dungeon_generate(&dungeon);
            break;
        default:
            usage(argv[0]);
            return 1;
    }    
    dungeon_print(&dungeon);

    djikstra_tunnel(&dungeon);
    djikstra_non_tunnel(&dungeon);
    printf("\nNon-Tunnel map\n");
    non_tunnel_map(&dungeon);
    printf("\nTunnel map\n");
    tunnel_map(&dungeon);
    

    delete_dungeon(&dungeon);

    return 0;
}

