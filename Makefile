TESTOBJS 	= blocks.o dir.o funcs.o metadata.o main.o tests.o dirlist.o difile.o
OBJS 	= blocks.o dir.o funcs.o metadata.o dirtest.o main.o tests.o dirlist.o difile.o
SOURCE	= main.c blocks.c dir.c funcs.c metadata.c dirtest.c tests.c dirlist.c difile.c
HEADER  = blocks.h dir.h funcs.h metadata.h tests.h dirlist.h difile.h
OUT  	= mydiz dirtest main
CC=gcc
DEBUG= -g
CFLAGS= -c -Wall $(DEBUG)
LFLAGS= -Wall $(DEBUG)

all: main $(SOURCE) $(HEADER)

dirtest: dirtest.o funcs.o metadata.o blocks.o
	$(CC) $(LFLAGS) dirtest.o funcs.o metadata.o blocks.o -o dirtest

main: $(TESTOBJS)
	$(CC) $(LFLAGS) $(TESTOBJS) -o main 

main.o: main.c
	$(CC) $(CFLAGS) main.c

dirlist.o: dirlist.c
	$(CC) $(CFLAGS) dirlist.c

difile.o: difile.c
	$(CC) $(CFLAGS) difile.c

dirtest.o: dirtest.c
	$(CC) $(CFLAGS) dirtest.c

metadata.o: metadata.c
	$(CC) $(CFLAGS) metadata.c

blocks.o: blocks.c
	$(CC) $(CFLAGS) blocks.c

dir.o: dir.c
	$(CC) $(CFLAGS) dir.c

funcs.o: funcs.c
	$(CC) $(CFLAGS) funcs.c

clean:
	rm -f $(OBJS) $(OUT)

count:
	wc $(SOURCE) $(HEADER)
