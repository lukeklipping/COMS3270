This project ports the Roguelike game from C to C++, introducing object-oriented design with pc and npc classes inheriting from character. 
The game now features a Fog of War system where the PC sees and remembers terrain within a light radius of 3, but monsters are only visible when illuminated. 
Debugging commands include toggling fog (f) and teleporting (g), allowing the PC to move instantly to a selected or random location. 
The makefile has been updated for C++ compilation, and rendering now supports visible and remembered terrain.


For TA:
    It works correctly, but if ran in valgrind, it throws memory errors