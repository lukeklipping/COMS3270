#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<endian.h>
#include<string.h>
#include<sys/stat.h>
#include<errno.h>

#include "dungeonGeneration.h"
#include "path.h"





//char dungeon[HEIGHT][WIDTH];
// fills dungeon with rock and initializes hardness
void dungeon_init(dungeon_t *d){
    
    int x, y;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT  - 1){
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
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            printf("%c", d->map[y][x]);
        }
        printf("\n");
    }
}

// checks if room is valid within bounds
int is_valid(dungeon_t *d,int x, int y, int w, int h){
    int i, j;
    if((x + w >= WIDTH - 1) || (y + h >= HEIGHT - 1)){
        return 0;
    }
    
    for(i = y - 2; i < y + h + 2; i++){
        for(j = x - 2; j < x + w + 2; j++){
            if(i >= 0 && j >= 0 && i < HEIGHT && j < WIDTH && d->map[i][j] == ROOM){
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

int save_stairs(dungeon_t *d, FILE *f){
    uint16_t numStairs;

    //up
    numStairs = htobe16(count_up_stairs(d));
    fwrite(&numStairs, 2, 1, f);
    int x,y;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(d->map[y][x] == '<'){
                numStairs--;
                fwrite(&x, 1, 1, f);
                fwrite(&y, 1, 1, f);
            }
        }
    }

    //down
    numStairs = htobe16(count_down_stairs(d));
    fwrite(&numStairs, 2, 1, f);
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(d->map[y][x] == '>'){
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
    int x, y, i = 0;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(d->map[y][x] == '<'){
                i++;
            }
        }
    }
    return i;
}

// helper method to count '>'
int count_down_stairs(dungeon_t *d){
    int x, y, i = 0;
    for(y = 0; y < HEIGHT; y++){
        for(x = 0; x < WIDTH; x++){
            if(d->map[y][x] == '>'){
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
            if(d->hardness[y][x] == 0){
                d->map[y][x] = HALL;
            } else if(d->hardness[y][x] == 255){
                d->map[y][x] = IMMUTABLE_WALL;
            } else{
                d->map[y][x] = ROCK;
                //could factor in immutable
            }
        }
    }
    return 0;
}

// reads rooms from file and places them into dungeon array
int read_rooms(dungeon_t *d, FILE *f){
    int i, y, x;
    uint16_t room_count;

    fread(&room_count, sizeof(room_count), 1, f);
    d->num_rooms = be16toh(room_count);

    d->rooms = malloc(sizeof(*d->rooms) * d->num_rooms);
    if (!d->rooms) {
        fprintf(stderr, "Memory allocation failed for rooms.\n");
        return 1;
    }

    for(i = 0; i < d->num_rooms; i++){
        uint8_t r;

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
                d->map[y][x] = ROOM;
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
        d->map[y][x] = '<';
    }
    //down
    fread(&stairs, 2, 1, f);
    stairs = be16toh(stairs);
    while(stairs--){
        fread(&x, 1, 1, f);
        fread(&y, 1, 1, f);
        d->map[y][x] = '>';
    }
    return 0;
}
int calculate_dungeon_size(dungeon_t *d){
  return (1708 + (d->num_rooms * 4) + (count_up_stairs(d) * 2) + (count_down_stairs(d) * 2));
}

// load dungeon
int load_dungeon(dungeon_t *d, char *file){
    char semantic[13];
    FILE *f = NULL;
    char *home;
    char *fileName;
    int length;
    uint32_t save_to32;

    if (!file) {
        if (!(home = getenv("HOME"))) {
            printf("undefined home\n");
            home = ".";
        }

        length = strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) + strlen(SAVE_DIR)+ 3;

        fileName = malloc(length * sizeof(*fileName));

        strcpy(fileName, home);
        strcat(fileName, "/.rlg327/dungeon.rlg327");

        f = fopen(fileName, "r");

        if (!f) {
            printf("fail to open %s\n", fileName);
            free(fileName);
            return 1;
        }
        free(fileName);
    } else {
        if (!(f = fopen(file, "r"))) {
            perror(file);
            exit(-1);
        }
    }

    d->num_rooms = 0;

    fread(semantic, sizeof(semantic) - 1, 1, f);
    semantic[12] = '\0';

    printf("Read semantic: '%s'\n", semantic); 

    if (strncmp(semantic, "RLG327-S2025", 12)) {
        fprintf(stderr, "Not an RLG327 save file.\n");
        exit(-1);
    }
    //file version
    fread(&save_to32, sizeof(save_to32), 1, f);
    //file size
    fread(&save_to32, sizeof(save_to32), 1, f);
    //save_to32 = be32toh(save_to32);

    fread(&d->PC.x, 1, 1, f);
    fread(&d->PC.y, 1, 1, f);

    read_dungeon_map(d, f);
    
    read_rooms(d, f);

    read_stairs(d, f);

    fclose(f);
    return 0;
}

// makes directory under home directory
//avoids duplicating directories
int makedirect(char *dir)
{
  char *slash;

  for (slash = dir + strlen(dir); slash > dir && *slash != '/'; slash--)
    ;

  if (slash == dir) {
    return 0;
  }
  *slash = '\0';

  if (mkdir(dir, 0700)) {
    if (errno != ENOENT && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
    if (*slash != '/') {
      return 1;
    }
    *slash = '\0';
    if (makedirect(dir)) {
      *slash = '/';
      return 1;
    }

    *slash = '/';
    if (mkdir(dir, 0700) && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
  }
    return 0;
}
int save_dungeon(dungeon_t *d, char *file){
    char *home;
    char *fileName = NULL;
    FILE *f = NULL;
    int dungeon_file_length;

    if(!file){
        if(!(home = getenv("HOME"))){
            printf("undefined home\n");
            home = ".";
        }
        dungeon_file_length = strlen(home) + strlen("/.rlg327") + strlen("/dungeon")+ 1; // +1 for the null byte
        fileName = malloc(dungeon_file_length * sizeof (*fileName));
        
        strcpy(fileName, home);
        strcat(fileName, "/.rlg327");

        makedirect(fileName);
        strcat(fileName, "/dungeon.rlg327");

        if (!(f = fopen(fileName, "wb"))) {
            perror(fileName);
            free(fileName);
            return 1;
        }
        free(fileName);
    } else {
        if (!(f = fopen(file, "w"))) {
        perror(file);
        exit(-1); // from prof code
        }
    }

    //printf("Dungeon saved successfully!\n");
    uint16_t write_to16;
    uint32_t write_to32;

    char *semantic = "RLG327-S2025";
    fwrite(semantic, 1, strlen(semantic), f);

    
    write_to32 = htobe32(DUNGEON_VERSIONX);
    fwrite(&write_to32, sizeof(write_to32), 1, f);

    //integer size file
    write_to32 = htobe32((uint32_t)calculate_dungeon_size(d));
    fwrite(&write_to32, sizeof (write_to32), 1, f);

    //A pair of unsigned 8-bit integers giving the x and y position of the PC
    uint8_t x = d->PC.x;
    uint8_t y = d->PC.y;
    fwrite(&x, 1, 1, f);
    fwrite(&y, 1, 1, f);
    
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j <WIDTH; j++){
            fwrite(&d->hardness[i][j], 1,1,f);
        }
    }
    
   //r, an unsigned 16-bit integer giving the number of rooms in the dungeon (2 bytes)
   write_to16 = htobe16((uint16_t)d->num_rooms);
   fwrite(&write_to16, sizeof(write_to16), 1, f);

    //rooms
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
    save_stairs(d,f);
    fclose(f);
    return 0;
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





void tunnel_map(dungeon_t *d){
    int i, j;

    for(i = 0; i < HEIGHT; i++){
        for(j = 0; j < WIDTH; j++){
            if(i == d->PC.y && j == d->PC.x) {
                printf("%c", PLAYER);
            } else if(d->PC_T[i][j] != 255){
                printf("%d", d->PC_T[i][j] % 10);
            } else{
                printf("%c", d->map[i][j]);
            }
           
        } 
    }
}

void non_tunnel_map(dungeon_t *d){
    int i, j;

    for(i = 0; i < HEIGHT; i++){
        for(j = 0; j < WIDTH; j++){
            if(i == d->PC.y && j == d->PC.x){
                printf("%c", PLAYER);
            } else if(d->PC_N[i][j] != 255){
                printf("%d", d->PC_N[i][j] % 10);
            } else{
                printf("%c", d->map[i][j]);
            }
        } 
    }
}


int delete_dungeon(dungeon_t *d){
    free(d->rooms);
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

