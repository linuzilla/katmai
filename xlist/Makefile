TARGET	= xlist.o
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c xlist.o
	$(CC) $(CFLAGS) -o $@ example.c xlist.o

clean:
	rm -f $(TARGET)
