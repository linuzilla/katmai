TARGET	= usec_timer.o example
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c usec_timer.o
	$(CC) $(CFLAGS) -o $@ example.c usec_timer.o

clean:
	rm -f $(TARGET)
