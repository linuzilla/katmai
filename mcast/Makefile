TARGET	= mcast.o example
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c mcast.o
	$(CC) $(CFLAGS) -o $@ example.c mcast.o

clean:
	rm -f $(TARGET)
