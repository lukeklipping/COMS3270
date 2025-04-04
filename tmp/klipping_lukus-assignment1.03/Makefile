

all: dungeonGeneration
	
dungeonGeneration: heap.o dungeonGeneration.o
	gcc -o dungeon dungeonGeneration.o heap.o

heap.o: heap.c heap.h
	gcc -ggdb -Wall -c heap.c

assign103.o:
	gcc -ggdb -Wall -c dungeonGeneration.c

clean:
	rm -f dungeon *~ *.o

#dungeon: dungeonGeneration.c dungeonGeneration.h
#	gcc dungeonGeneration.c -o dungeon -Wall -Werror
#clean:
#	rm -f dungeon *~ *.d