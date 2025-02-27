all: dungeon
	
dungeon: rlg.o heap.o dungeonGeneration.o path.o readWriteDungeon.o character.o monster.o eventSim.o
	gcc -o dungeon rlg.o heap.o dungeonGeneration.o path.o readWriteDungeon.o character.o monster.o eventSim.o

rlg.o: rlg.c
	gcc -ggdb -Wall -c rlg.c

heap.o: heap.c heap.h
	gcc -ggdb -Wall -c heap.c

dungeonGeneration.o: dungeonGeneration.c dungeonGeneration.h
	gcc -ggdb -Wall -c dungeonGeneration.c

path.o: path.c path.h
	gcc -ggdb -Wall -c path.c

readWriteDungeon.o: readWriteDungeon.c readWriteDungeon.h
	gcc -ggdb -Wall -c readWriteDungeon.c

character.o: character.c character.h
	gcc -ggdb -Wall -c character.c

monster.o: monster.c monster.h
	gcc -ggdb -Wall -c monster.c

eventSim.o: eventSim.c eventSim.h
	gcc -ggdb -Wall -c eventSim.c

clean:
	rm -f dungeon *~ *.o