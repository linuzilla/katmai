TARGET	= pqueue.o
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c pqueue.o
	$(CC) $(CFLAGS) -o $@ example.c pqueue.o

clean:
	rm -f $(TARGET)
