OBJS 	= block.o dilists.o list_main.o block_main.o dir.o funcs.o metadata.o dirtest.o
SOURCE	= block_main.c block.c dilists.c dinode.c dir.c funcs.c metadata.c dirtest.c
HEADER  = block.h dilists.h dinode.h funcs.h metadata.h 
OUT  	= mydiz metamain dir blktest list_main dirtest
CC	= gcc
FLAGS   = -g -c 

all: blktest $(SOURCE) $(HEADER)

dirtest: dirtest.o funcs.o metadata.o block.o
	$(CC) -g dirtest.o funcs.o metadata.o block.o -o dirtest

metamain: metamain.o metadata.o
	$(CC) -g metamain.o metadata.o -o metamain

dir: dir.o funcs.o
	$(CC) -g dir.o funcs.o -o dir

blktest: block_main.o blocks.o 
	$(CC) -g block_main.o blocks.o -o blktest

list_main: list_main.o dilists.o
	$(CC) -g list_main.o dilists.o -o list_main

dirtest.o: dirtest.c
	$(CC) $(FLAGS) dirtest.c

metamain.o: metamain.c
	$(CC) $(FLAGS) metamain.c

metadata.o: metadata.c
	$(CC) $(FLAGS) metadata.c

block_main.o: block_main.c
	$(CC) $(FLAGS) block_main.c

list_main.o: list_main.c
	$(CC) $(FLAGS) list_main.c

dilists.o: dilists.c
	$(CC) $(FLAGS) dilists.c

blocks.o: blocks.c
	$(CC) $(FLAGS) blocks.c

dir.o: dir.c
	$(CC) $(FLAGS) dir.c

funcs.o: funcs.c
	$(CC) $(FLAGS) funcs.c

clean:
	rm -f $(OBJS) $(OUT)

count:
	wc $(SOURCE) $(HEADER)
