all: dungeonGeneration
	
dungeonGeneration: heap.o dungeonGeneration.o path.o readWriteDungeon.o
	gcc -o dungeon dungeonGeneration.o heap.o path.o readWriteDungeon.o

heap.o: heap.c heap.h
	gcc -ggdb -Wall -c heap.c

assign103.o:
	gcc -ggdb -Wall -c dungeonGeneration.c

path.o: path.c path.h
	gcc -ggdb -Wall -c path.c

readWrite.o: readWriteDungeon.c readWriteDungeon.h
	gcc -ggdb -Wall -c readWriteDungeon.c

clean:
	rm -f dungeon *~ *.o

#dungeon: dungeonGeneration.c dungeonGeneration.h
#	gcc dungeonGeneration.c -o dungeon -Wall -Werror
#clean:
#	rm -f dungeon *~ *.d