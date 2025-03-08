#ifndef PATH_H
#define PATH_H

#include "heap.h"

struct dungeon_t;
typedef struct dungeon_t dungeon_t;
struct pc_t;
typedef struct pc_t pc_t;

typedef struct path
{
    heap_node_t *heapNode;
    int xP;
    int yP;
} path_t;

int hardness_weight(dungeon_t *d, int, int);
void djikstra_tunnel(dungeon_t *d);
void djikstra_non_tunnel(dungeon_t *d);
void tunnel_map(dungeon_t *d);
void non_tunnel_map(dungeon_t *d);

#endif