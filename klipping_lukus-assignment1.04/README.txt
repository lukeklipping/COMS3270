New files
    character.c, character.h, monster.c, monster.h, path.c. path.h, readWriteDungeon.c, readWriteDungeon.h, eventSim.c, eventSim.h, rlg.c

    This iteration of the RLG dungeon, I decided to split up most of the methods into their corresponding files to match. I believed 
the original file was becoming to large to manage and read, with lots of scrolling, so I split them up. I made a new character.h and .c
to represent characters with their variables, and monster.h and monster.c for a monsters variables and move functions. For the 16 
different comibnations of monsters, 


Currently, my PC does not move, which is why I made the 'q to quit' function in eventSim. Just in case the monsters can't find the PC.



For TA:

My implementation works about 85% of the time. Sometimes it hits a segfault or freezes. But it works the other times.