
#include <endian.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "character.h"
#include "readWriteDungeon.h"

// helper method to count '<'
static int count_up_stairs(dungeon_t *d)
{
    int x, y, i = 0;
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {
            if (d->map[y][x] == '<')
            {
                i++;
            }
        }
    }
    return i;
}

// helper method to count '>'
static int count_down_stairs(dungeon_t *d)
{
    int x, y, i = 0;
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {
            if (d->map[y][x] == '>')
            {
                i++;
            }
        }
    }
    return i;
}

// saves stairs
int save_stairs(dungeon_t *d, FILE *f)
{
    uint16_t numStairs;

    // up
    numStairs = htobe16(count_up_stairs(d));
    fwrite(&numStairs, 2, 1, f);
    int x, y;
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {
            if (d->map[y][x] == '<')
            {
                numStairs--;
                fwrite(&x, 1, 1, f);
                fwrite(&y, 1, 1, f);
            }
        }
    }

    // down
    numStairs = htobe16(count_down_stairs(d));
    fwrite(&numStairs, 2, 1, f);
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {
            if (d->map[y][x] == '>')
            {
                numStairs--;
                fwrite(&x, 1, 1, f);
                fwrite(&y, 1, 1, f);
            }
        }
    }
    return 0;
}

// fileread helper to place dungeon map
int read_dungeon_map(dungeon_t *d, FILE *f)
{
    int y, x;

    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {
            fread(&d->hardness[y][x], 1, 1, f);
            if (d->hardness[y][x] == 0)
            {
                d->map[y][x] = HALL;
            }
            else if (d->hardness[y][x] == 255)
            {
                d->map[y][x] = IMMUTABLE_WALL;
            }
            else
            {
                d->map[y][x] = ROCK;
                // could factor in immutable
            }
        }
    }
    return 0;
}

// reads rooms from file and places them into dungeon array
int read_rooms(dungeon_t *d, FILE *f)
{
    int i, y, x;
    uint16_t room_count;

    fread(&room_count, sizeof(room_count), 1, f);
    d->num_rooms = be16toh(room_count);

    d->rooms = malloc(sizeof(*d->rooms) * d->num_rooms);
    if (!d->rooms)
    {
        fprintf(stderr, "Memory allocation failed for rooms.\n");
        return 1;
    }

    for (i = 0; i < d->num_rooms; i++)
    {
        uint8_t r;

        fread(&r, 1, 1, f);
        d->rooms[i].x = r;
        fread(&r, 1, 1, f);
        d->rooms[i].y = r;
        fread(&r, 1, 1, f);
        d->rooms[i].width = r;
        fread(&r, 1, 1, f);
        d->rooms[i].height = r;

        for (y = d->rooms[i].y; y < d->rooms[i].y + d->rooms[i].height; y++)
        {
            for (x = d->rooms[i].x; x < d->rooms[i].x + d->rooms[i].width; x++)
            {
                d->map[y][x] = ROOM;
            }
        }
    }
    return 0;
}

// helper method to read stairs and place into dungeon
int read_stairs(dungeon_t *d, FILE *f)
{
    uint16_t stairs;
    uint8_t x, y;

    fread(&stairs, 2, 1, f);
    stairs = be16toh(stairs);

    // up
    while (stairs--)
    {
        fread(&x, 1, 1, f);
        fread(&y, 1, 1, f);
        d->map[y][x] = '<';
    }
    // down
    fread(&stairs, 2, 1, f);
    stairs = be16toh(stairs);
    while (stairs--)
    {
        fread(&x, 1, 1, f);
        fread(&y, 1, 1, f);
        d->map[y][x] = '>';
    }
    return 0;
}
int calculate_dungeon_size(dungeon_t *d)
{
    return (1708 + (d->num_rooms * 4) + (count_up_stairs(d) * 2) + (count_down_stairs(d) * 2));
}

// load dungeon
int load_dungeon(dungeon_t *d, char *file)
{
    char semantic[13];
    FILE *f = NULL;
    char *home;
    char *fileName;
    int length;
    uint32_t save_to32;

    if (!file)
    {
        if (!(home = getenv("HOME")))
        {
            printf("undefined home\n");
            home = ".";
        }

        length = strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) + strlen(SAVE_DIR) + 3;

        fileName = malloc(length * sizeof(*fileName));

        strcpy(fileName, home);
        strcat(fileName, "/.rlg327/dungeon.rlg327");

        f = fopen(fileName, "r");

        if (!f)
        {
            printf("fail to open %s\n", fileName);
            free(fileName);
            return 1;
        }
        free(fileName);
    }
    else
    {
        if (!(f = fopen(file, "r")))
        {
            perror(file);
            exit(-1);
        }
    }

    d->num_rooms = 0;

    fread(semantic, sizeof(semantic) - 1, 1, f);
    semantic[12] = '\0';

    printf("Read semantic: '%s'\n", semantic);

    if (strncmp(semantic, "RLG327-S2025", 12))
    {
        fprintf(stderr, "Not an RLG327 save file.\n");
        exit(-1);
    }
    // file version
    fread(&save_to32, sizeof(save_to32), 1, f);
    // file size
    fread(&save_to32, sizeof(save_to32), 1, f);
    // save_to32 = be32toh(save_to32);

    fread(&d->PC->position.x, 1, 1, f);
    fread(&d->PC->position.y, 1, 1, f);

    read_dungeon_map(d, f);

    read_rooms(d, f);

    read_stairs(d, f);

    fclose(f);
    return 0;
}

// makes directory under home directory
// avoids duplicating directories
int makedirect(char *dir)
{
    char *slash;

    for (slash = dir + strlen(dir); slash > dir && *slash != '/'; slash--)
        ;

    if (slash == dir)
    {
        return 0;
    }
    *slash = '\0';

    if (mkdir(dir, 0700))
    {
        if (errno != ENOENT && errno != EEXIST)
        {
            fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
            return 1;
        }
        if (*slash != '/')
        {
            return 1;
        }
        *slash = '\0';
        if (makedirect(dir))
        {
            *slash = '/';
            return 1;
        }

        *slash = '/';
        if (mkdir(dir, 0700) && errno != EEXIST)
        {
            fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
            return 1;
        }
    }
    return 0;
}
// saves dungeons
int save_dungeon(dungeon_t *d, char *file)
{
    char *home;
    char *fileName = NULL;
    FILE *f = NULL;
    int dungeon_file_length;

    if (!file)
    {
        if (!(home = getenv("HOME")))
        {
            printf("undefined home\n");
            home = ".";
        }
        dungeon_file_length = strlen(home) + strlen("/.rlg327") + strlen("/dungeon") + 1; // +1 for the null byte
        fileName = malloc(dungeon_file_length * sizeof(*fileName));

        strcpy(fileName, home);
        strcat(fileName, "/.rlg327");

        makedirect(fileName);
        strcat(fileName, "/dungeon.rlg327");

        if (!(f = fopen(fileName, "wb")))
        {
            perror(fileName);
            free(fileName);
            return 1;
        }
        free(fileName);
    }
    else
    {
        if (!(f = fopen(file, "w")))
        {
            perror(file);
            exit(-1); // from prof code
        }
    }

    // printf("Dungeon saved successfully!\n");
    uint16_t write_to16;
    uint32_t write_to32;

    char *semantic = "RLG327-S2025";
    fwrite(semantic, 1, strlen(semantic), f);

    write_to32 = htobe32(DUNGEON_VERSIONX);
    fwrite(&write_to32, sizeof(write_to32), 1, f);

    // integer size file
    write_to32 = htobe32((uint32_t)calculate_dungeon_size(d));
    fwrite(&write_to32, sizeof(write_to32), 1, f);

    // A pair of unsigned 8-bit integers giving the x and y position of the PC
    uint8_t x = d->PC->position.x;
    uint8_t y = d->PC->position.y;
    fwrite(&x, 1, 1, f);
    fwrite(&y, 1, 1, f);

    for (int i = 0; i < DUNGEON_Y; i++)
    {
        for (int j = 0; j < DUNGEON_X; j++)
        {
            fwrite(&d->hardness[i][j], 1, 1, f);
        }
    }

    // r, an unsigned 16-bit integer giving the number of rooms in the dungeon (2 bytes)
    write_to16 = htobe16((uint16_t)d->num_rooms);
    fwrite(&write_to16, sizeof(write_to16), 1, f);

    // rooms
    for (int i = 0; i < d->num_rooms; i++)
    {
        uint8_t xpos = d->rooms[i].x;
        uint8_t ypos = d->rooms[i].y;
        uint8_t xdim = d->rooms[i].width;
        uint8_t ydim = d->rooms[i].height;

        fwrite(&xpos, sizeof(uint8_t), 1, f);
        fwrite(&ypos, sizeof(uint8_t), 1, f);
        fwrite(&xdim, sizeof(uint8_t), 1, f);
        fwrite(&ydim, sizeof(uint8_t), 1, f);
    }
    save_stairs(d, f);
    fclose(f);
    return 0;
}