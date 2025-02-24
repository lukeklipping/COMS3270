#include<stdio.h>
#include "path.h"

// could not find a better way for cmp pointer
static dungeon_t *dun_cmp = NULL;

// heap uses int32_t
// non tunnel
static int32_t compare_distance(const void *key, const void *with) {
  return ((int32_t) dun_cmp->PC_N[((path_t *) key)->yP][((path_t *) key)->xP] -
          (int32_t) dun_cmp->PC_N[((path_t *) with)->yP][((path_t *) with)->xP]);
}


//
static int32_t compare_tunnel_distance(const void *key, const void *with) {
  return ((int32_t) dun_cmp->PC_T[((path_t *) key)->yP][((path_t *) key)->xP] -
          (int32_t) dun_cmp->PC_T[((path_t *) with)->yP][((path_t *) with)->xP]);
}
/*
non_tunnel monsters can only go through '.','#', and '@'
*/
// djikstra algo for non tunneling monsters
void djikstra_non_tunnel(dungeon_t *d){
    heap_t heap;
    static path_t path[DUNGEON_Y][DUNGEON_X], *tmp;
    int i, j;
    int init = 0;

    if(!init){
        init = 1;
        dun_cmp = d;
        // initialize tunnel path map
        for(i = 0; i < DUNGEON_Y; i++){
            for(j = 0; j < DUNGEON_X; j++){
                path[i][j].yP = i;
                path[i][j].xP = j;
            }
        }
    }
    // initialize @ grid
    for(i = 0; i < DUNGEON_Y; i++){
        for(j = 0; j < DUNGEON_X; j++){
            d->PC_N[i][j] = 255;
        }
    }
    d->PC_N[d->PC.y][d->PC.x] = 0;

    heap_init(&heap, compare_distance, NULL);
    
    // insert into heap 
    for(i = 0; i < DUNGEON_Y; i++){
        for(j = 0; j < DUNGEON_X; j++){
            //insert into heap if walkable character
            //otherwise set null
            if(d->map[i][j] == ROOM || d->map[i][j] == HALL || d->map[i][j] == PLAYER  || d->map[i][j] == '>' || d->map[i][j] == '<'){
                path[i][j].heapNode = heap_insert(&heap, &path[i][j]);
            }
        } 
    }
    

        while((tmp = heap_remove_min(&heap))){
        // set min node to null removing from heap
        tmp->heapNode = NULL;

        // explore all 8 neighbors

        // top left neighbor cell
        // compares current distance to new calculated distance
        if ((path[tmp->yP - 1][tmp->xP - 1].heapNode) && 
            (d->PC_N[tmp->yP - 1][tmp->xP - 1] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP - 1][tmp->xP - 1] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP - 1][tmp->xP - 1].heapNode);
        }
        // top
        if ((path[tmp->yP - 1][tmp->xP].heapNode) && 
            (d->PC_N[tmp->yP - 1][tmp->xP] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP - 1][tmp->xP] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP - 1][tmp->xP].heapNode);
        }
        // top right
        if ((path[tmp->yP - 1][tmp->xP + 1].heapNode) && 
            (d->PC_N[tmp->yP - 1][tmp->xP + 1] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP - 1][tmp->xP + 1] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP - 1][tmp->xP + 1].heapNode);
        }
        // left
        if ((path[tmp->yP][tmp->xP - 1].heapNode) && 
            (d->PC_N[tmp->yP][tmp->xP - 1] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP][tmp->xP - 1] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP][tmp->xP - 1].heapNode);
        }
        // right
        if ((path[tmp->yP][tmp->xP + 1].heapNode) && 
            (d->PC_N[tmp->yP][tmp->xP + 1] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP][tmp->xP + 1] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP][tmp->xP + 1].heapNode);
        }
        // bottom left
        if ((path[tmp->yP + 1][tmp->xP - 1].heapNode) && 
            (d->PC_N[tmp->yP + 1][tmp->xP - 1] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP + 1][tmp->xP - 1] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP + 1][tmp->xP - 1].heapNode);
        }
        // bottom
        if ((path[tmp->yP + 1][tmp->xP].heapNode) && 
            (d->PC_N[tmp->yP + 1][tmp->xP] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP + 1][tmp->xP] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP + 1][tmp->xP].heapNode);
        }
        // bottom right
        if ((path[tmp->yP + 1][tmp->xP + 1].heapNode) && 
            (d->PC_N[tmp->yP + 1][tmp->xP + 1] >
            d->PC_N[tmp->yP][tmp->xP] + 1)) {
                d->PC_N[tmp->yP + 1][tmp->xP + 1] = d->PC_N[tmp->yP][tmp->xP] + 1;
                heap_decrease_key_no_replace(&heap, path[tmp->yP + 1][tmp->xP + 1].heapNode);
        }
    }
    //delete at end to free 
    heap_delete(&heap);
}

// hardness  = 1 + h / 85
int hardness_weight(dungeon_t *d, int x, int y){
    return((d->hardness[y][x] / 85) + 1);
}

//djikstra algorithm for tunneling monsters
void djikstra_tunnel(dungeon_t *d){
    heap_t heap;
    static path_t path[DUNGEON_Y][DUNGEON_X];
    path_t *tmp;
    int i, j;
    int init = 0;

    if(!init){
        init = 1;
        dun_cmp = d;
        // initialize tunnel path map
        for(i = 0; i < DUNGEON_Y; i++){
            for(j = 0; j < DUNGEON_X; j++){
                path[i][j].yP = i;
                path[i][j].xP = j;
            }
        }
    }
    

    for(i = 0; i < DUNGEON_Y; i++){
        for(j = 0; j < DUNGEON_X; j++){
            d->PC_T[i][j] = 255;
        }
    }
    //sets pc pos
    d->PC_T[d->PC.y][d->PC.x] = 0;

    heap_init(&heap, compare_tunnel_distance, NULL);
    
    // insert into heap 
    for(i = 0; i < DUNGEON_Y; i++){
        for(j = 0; j < DUNGEON_X; j++){
            //insert into heap if wall is not immutable (255)
            if(d->hardness[i][j] != 255){
                path[i][j].heapNode = heap_insert(&heap, &path[i][j]);
            }
        }
    }


    while((tmp = heap_remove_min(&heap))){
        // set min node to null removing from heap
        tmp->heapNode = NULL;

        //explore all 8 neighbors
        // top left
        // top left
        if ((path[tmp->yP - 1][tmp->xP - 1].heapNode) && 
            (d->PC_T[tmp->yP - 1][tmp->xP - 1] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP - 1][tmp->xP - 1] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP - 1][tmp->xP - 1].heapNode);
        }

        // top
        //checks if valid entry in heapNode
        // if path[y - 1][x - 1] in node && pTunnel[y-1][x-1] > pTunnel[y][x] + hardness
            // decrease heap at that location
        if ((path[tmp->yP - 1][tmp->xP].heapNode) && 
            (d->PC_T[tmp->yP - 1][tmp->xP] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP - 1][tmp->xP] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP - 1][tmp->xP].heapNode);
        }

        // top right
        if ((path[tmp->yP - 1][tmp->xP + 1].heapNode) && 
            (d->PC_T[tmp->yP - 1][tmp->xP + 1] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP - 1][tmp->xP + 1] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP - 1][tmp->xP + 1].heapNode);
        }

        // left
        if ((path[tmp->yP][tmp->xP - 1].heapNode) && 
            (d->PC_T[tmp->yP][tmp->xP - 1] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP][tmp->xP - 1] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP][tmp->xP - 1].heapNode);
        }

        // right
        if ((path[tmp->yP][tmp->xP + 1].heapNode) && 
            (d->PC_T[tmp->yP][tmp->xP + 1] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP][tmp->xP + 1] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP][tmp->xP + 1].heapNode);
        }

        // bottom left
        if ((path[tmp->yP + 1][tmp->xP - 1].heapNode) && 
            (d->PC_T[tmp->yP + 1][tmp->xP - 1] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP + 1][tmp->xP - 1] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP + 1][tmp->xP - 1].heapNode);
        }

        // bottom
        if ((path[tmp->yP + 1][tmp->xP].heapNode) && 
            (d->PC_T[tmp->yP + 1][tmp->xP] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP + 1][tmp->xP] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP + 1][tmp->xP].heapNode);
        }

        // bottom right
        if ((path[tmp->yP + 1][tmp->xP + 1].heapNode) && 
            (d->PC_T[tmp->yP + 1][tmp->xP + 1] > 
            d->PC_T[tmp->yP][tmp->xP] + hardness_weight(d, tmp->xP, tmp->yP))) {
                d->PC_T[tmp->yP + 1][tmp->xP + 1] = d->PC_T[tmp->yP][tmp->xP] + 
                hardness_weight(d, tmp->xP, tmp->yP);
                heap_decrease_key_no_replace(&heap, path[tmp->yP + 1][tmp->xP + 1].heapNode);
        }
   
    }
    //delete at end to free 
    heap_delete(&heap);
}

void tunnel_map(dungeon_t *d){
    int i, j;

    for(i = 0; i < DUNGEON_Y; i++){
        for(j = 0; j < DUNGEON_X; j++){
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

    for(i = 0; i < DUNGEON_Y; i++){
        for(j = 0; j < DUNGEON_X; j++){
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