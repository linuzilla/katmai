TARGET	= mqueue.o
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c mqueue.o
	$(CC) $(CFLAGS) -o $@ example.c mqueue.o

clean:
	rm -f $(TARGET)
