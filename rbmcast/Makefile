TARGET	= example rbmcast.o
CC=gcc
CFLAGS+=-Wall -I. -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c rbmcast.o ../libkatmai.a
	$(CC) $(CFLAGS) -o $@ example.c rbmcast.o ../libkatmai.a

clean:
	rm -f $(TARGET)
