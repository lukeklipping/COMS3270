#CFLAGS = -g -ggdb -Wall  #debugging
CFLAGS = -Werror -Wall  

all: dungeon

dungeon: rlg.o heap.o dungeonGeneration.o path.o readWriteDungeon.o character.o monster.o eventSim.o pc.o move.o io.o
	gcc $(CFLAGS) -o dungeon rlg.o heap.o dungeonGeneration.o path.o readWriteDungeon.o character.o monster.o eventSim.o pc.o move.o io.o

rlg.o: rlg.c
	gcc $(CFLAGS) -c rlg.c

heap.o: heap.c heap.h
	gcc $(CFLAGS) -c heap.c

dungeonGeneration.o: dungeonGeneration.c dungeonGeneration.h
	gcc $(CFLAGS) -c dungeonGeneration.c

path.o: path.c path.h
	gcc $(CFLAGS) -c path.c

readWriteDungeon.o: readWriteDungeon.c readWriteDungeon.h
	gcc $(CFLAGS) -c readWriteDungeon.c

character.o: character.c character.h
	gcc $(CFLAGS) -c character.c

monster.o: monster.c monster.h
	gcc $(CFLAGS) -c monster.c

eventSim.o: eventSim.c eventSim.h
	gcc $(CFLAGS) -c eventSim.c

pc.o: pc.c pc.h
	gcc $(CFLAGS) -c pc.c

move.o: move.c move.h
	gcc $(CFLAGS) -c move.c

io.o: io.c io.h
	gcc $(CFLAGS) -c io.c


clean:
	rm -f dungeon *~ *.o