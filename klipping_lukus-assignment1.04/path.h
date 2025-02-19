#ifndef PATH_H
#define PATH_H

#include "dungeonGeneration.h"

typedef struct path{
    heap_node_t *heapNode;
    int xP;
    int yP;
} path_t;

int hardness_weight(dungeon_t *d, int, int);
void djikstra_tunnel(dungeon_t *d);
void djikstra_non_tunnel(dungeon_t *d);


#endif