TARGET	= aumixer.o example
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c aumixer.o
	$(CC) $(CFLAGS) -o $@ example.c aumixer.o

clean:
	rm -f $(TARGET)
