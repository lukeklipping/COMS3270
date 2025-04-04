New files
    heap.c & heap.h
New methods
    djikstra_tunnel(), djikstra_non_tunnel(), tunnel_map(), non_tunnel_map(), delete_dungeon(), hardness_weight()

    This iteration of the roguelike dungeon assignment brings in pathfinding into the loop. Using Djikstras algorithm
and a heap. It calculates the distance between the 'monster' (soon to come) and the Player char. It chooses a neighboring 
cell with the lowest cost. This includes non tunneling monsters and tunneling monsters. For tunneling monsters, this is 
when hardness comes into play. The algorithm will choose the path with the lower hardness_weight to go to.


For TA:
I believe my implementation to be correct. Makefile should compile all necessary elements.