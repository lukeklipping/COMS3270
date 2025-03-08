#ifndef PC_H
#define PC_H

struct dungeon_t;
typedef struct dungeon_t dungeon_t;

typedef struct pc_t
{
    // future attacks ??
} pc_t;

void pc_delete(pc_t *p);
int pc_alive(dungeon_t *d);
void pc_place(dungeon_t *d);
int pc_generate(dungeon_t *d);
int pc_within_room(dungeon_t *d, int);

#endif