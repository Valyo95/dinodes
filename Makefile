OBJS 	= blocks.o dilists.o list_main.o blk_main.o
SOURCE	= blk_main.c blocks.c dilists.c dinode.c
HEADER  = blocks.h dilists.h dinode.h
OUT  	= mydiz
CC	= gcc
FLAGS   = -g -c 

all: blktest $(SOURCE) $(HEADER)

blktest: blk_main.o blocks.o 
	$(CC) -g blk_main.o blocks.o -o blktest

blk_main.o: blk_main.c
	$(CC) $(FLAGS) blk_main.c

list_main: list_main.o dilists.o
	$(CC) -g list_main.o dilists.o -o list_main

list_main.o: list_main.c
	$(CC) $(FLAGS) list_main.c

dilists.o: dilists.c
	$(CC) $(FLAGS) dilists.c

blocks.o: blocks.c
	$(CC) $(FLAGS) blocks.c

clean:
	rm -f $(OBJS) $(OUT)

count:
	wc $(SOURCE) $(HEADER)
