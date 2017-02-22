OBJS 	= blocks.o metadata.o main.o dirlist.o difile.o
SOURCE	= main.c blocks.c metadata.c dirlist.c difile.c
HEADER  = blocks.h metadata.h dirlist.h difile.h
OUT  	= mydiz
CC=gcc
DEBUG= -g
CFLAGS= -c -Wall $(DEBUG)
LFLAGS= -Wall $(DEBUG)

all: mydiz $(SOURCE) $(HEADER)

dirtest: dirtest.o metadata.o blocks.o
	$(CC) $(LFLAGS) metadata.o blocks.o -o dirtest

mydiz: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o mydiz

main.o: main.c
	$(CC) $(CFLAGS) main.c

dirlist.o: dirlist.c
	$(CC) $(CFLAGS) dirlist.c

difile.o: difile.c
	$(CC) $(CFLAGS) difile.c

metadata.o: metadata.c
	$(CC) $(CFLAGS) metadata.c

blocks.o: blocks.c
	$(CC) $(CFLAGS) blocks.c

clean:
	rm -f $(OBJS) $(OUT)

count:
	wc $(SOURCE) $(HEADER)
