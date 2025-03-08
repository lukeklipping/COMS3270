#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "path.h"
#include "dungeonGeneration.h"
#include "character.h"
#include "readWriteDungeon.h"

// char dungeon[DUNGEON_Y][DUNGEON_X];
//  fills dungeon with rock and initializes hardness
void dungeon_init(dungeon_t *d)
{
    dungeon_empty_init(d);
}

void dungeon_empty_init(dungeon_t *d)
{

    int x, y;
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {
            if (x == 0 || y == 0 || x == DUNGEON_X - 1 || y == DUNGEON_Y - 1)
            {
                d->map[y][x] = ROCK;
                d->hardness[y][x] = IMMUTABLE_WALL;
            }
            else
            {
                d->map[y][x] = ROCK;
                d->hardness[y][x] = 1 + rand() % 254;
            }
            d->character[y][x] = NULL;
        }
    }
}
// prints dungeon
void dungeon_print(dungeon_t *d)
{
    int x, y;
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {

            printf("%c", d->map[y][x]);
        }
        printf("\n");
    }
}

// checks if room is valid within bounds
int is_valid(dungeon_t *d, int x, int y, int w, int h)
{
    int i, j;
    if ((x + w >= DUNGEON_X - 2) || (y + h >= DUNGEON_Y - 2))
    {
        return 0;
    }

    for (i = y - 2; i < y + h + 2; i++)
    {
        for (j = x - 2; j < x + w + 2; j++)
        {
            if (i >= 0 && j >= 0 && i < DUNGEON_Y && j < DUNGEON_X && d->map[i][j] == ROOM)
            {
                return 0;
            }
        }
    }
    return 1;
}

// generates rooms and returns room count and places character
void generate_rooms(dungeon_t *d)
{
    int num_rooms = MIN_ROOMS + rand() % (MAX_ROOMS - MIN_ROOMS + 1);
    int room_count = 0;
    int j, i;

    // dungeon_t *d = malloc(sizeof(dungeon_t));
    d->rooms = malloc(num_rooms * sizeof(Room));
    d->num_rooms = 0;

    while (room_count < num_rooms)
    {
        int width = MIN_DUNGEON_X + rand() % 12;
        // printf("%d", width);
        int height = MIN_DUNGEON_Y + rand() % 8;
        // printf("%d", height);

        // keeps outermost barrier
        int x = 1 + rand() % (DUNGEON_X - width - 3);
        int y = 1 + rand() % (DUNGEON_Y - height - 3);

        if (is_valid(d, x, y, width, height))
        {
            for (i = y; i < y + height; i++)
            {
                for (j = x; j < x + width; j++)
                {
                    d->map[i][j] = ROOM;
                    d->hardness[i][j] = 0;
                }
            }
            d->rooms[room_count].x = x;
            d->rooms[room_count].y = y;
            d->rooms[room_count].width = width;
            d->rooms[room_count].height = height;

            room_count++;
        }
    }
    d->num_rooms = room_count;
}

// generates hallways from room to room, continues past room characters
void generate_corridor(dungeon_t *d)
{
    int i;
    for (i = 0; i < d->num_rooms - 1; i++)
    {
        // printf("%d %d %d %d", (*r)[i].x, (*r)[i].y, (*r)[i].width, (*r)[i].height);
        int x1 = d->rooms[i].x + d->rooms[i].width / 2;
        int y1 = d->rooms[i].y + d->rooms[i].height / 2;
        int x2 = d->rooms[i + 1].x + d->rooms[i + 1].width / 2;
        int y2 = d->rooms[i + 1].y + d->rooms[i + 1].height / 2;

        // horizontal corridor
        if (x1 < x2)
        {
            for (int x = x1; x <= x2; x++)
            {
                if (d->map[y1][x] != ROOM)
                {
                    d->map[y1][x] = HALL;
                    d->hardness[y1][x] = 0;
                }
            }
        }
        else
        {
            for (int x = x1; x >= x2; x--)
            {
                if (d->map[y1][x] != ROOM)
                {
                    d->map[y1][x] = HALL;
                    d->hardness[y1][x] = 0;
                }
            }
        }

        // vertical corridor
        if (y1 < y2)
        {
            for (int y = y1; y <= y2; y++)
            {
                if (d->map[y][x2] != ROOM)
                {
                    d->map[y][x2] = HALL;
                    d->hardness[y][x2] = 0;
                }
            }
        }
        else
        {
            for (int y = y1; y >= y2; y--)
            {
                if (d->map[y][x2] != ROOM)
                {
                    d->map[y][x2] = HALL;
                    d->hardness[y][x2] = 0;
                }
            }
        }
    }
}
// generates both stairs
void generate_stairs(dungeon_t *d)
{
    //< up
    //> down
    char stairs[2] = {'<', '>'};
    int i, y, x;
    // d->stairs = malloc(2 * sizeof(pair_t));

    for (i = 0; i < 2; i++)
    {
        int room_index = rand() % d->num_rooms;
        do
        {
            y = d->rooms[room_index].y + (rand() % d->rooms[room_index].height);
            x = d->rooms[room_index].x + (rand() % d->rooms[room_index].width);
        } while (d->map[y][x] != ROOM);
        // d->stairs[i].x = x;
        // d->stairs[i].y = y;
        d->map[y][x] = stairs[i];
        d->hardness[y][x] = 0;
    }
    // d->num_up_stairs++;
    // d->num_down_stairs++;
}

// frees dungeon variables
void delete_dungeon(dungeon_t *d)
{
    // free(d->character);
    free(d->rooms);
}

void delete_characterArray(dungeon_t *d)
{
    for (int i = 0; i < DUNGEON_Y; i++)
    {
        for (int j = 0; j < DUNGEON_X; j++)
        {
            if (d->character[i][j])
            {
                if (d->character[i][j]->mon_character)
                {
                    // pc in here
                    free(d->character[i][j]->mon_character);
                    d->character[i][j]->mon_character = NULL;
                }
                free(d->character[i][j]);
                d->character[i][j] = NULL;
            }
        }
    }
}

void copy_to_terrain(dungeon_t *d)
{
    int x, y;
    for (y = 0; y < DUNGEON_Y; y++)
    {
        for (x = 0; x < DUNGEON_X; x++)
        {

            d->terrain[y][x] = d->map[y][x];
        }
    }
}
