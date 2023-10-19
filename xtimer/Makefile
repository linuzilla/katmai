TARGET	= xtimer.o
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c xtimer.o
	$(CC) $(CFLAGS) -o $@ example.c xtimer.o

clean:
	rm -f $(TARGET)
