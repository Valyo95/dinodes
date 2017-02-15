OBJS 	= blk_main.o blocks.o
SOURCE	= blk_main.c blocks.c
HEADER  = blocks.h
OUT  	= mydiz
CC	= gcc
FLAGS   = -g -c 

all: blktest $(SOURCE) $(HEADER)

blktest: blk_main.o blocks.o 
	$(CC) -g blk_main.o blocks.o -o blktest

blk_main.o: blk_main.c
	$(CC) $(FLAGS) blk_main.c

blocks.o: blocks.c
	$(CC) $(FLAGS) blocks.c

clean:
	rm -f $(OBJS) $(OUT)

count:
	wc $(SOURCE) $(HEADER)
