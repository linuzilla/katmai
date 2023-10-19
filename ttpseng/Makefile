TARGET	= ttpseng.o example
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread
CFLAGS+=-I /usr/local/include/freetype2
LIBS=-lfreetype -lz
LFLAGS=-L/usr/local/lib

all:	$(TARGET)

example:	example.c ttpseng.o
	$(CC) $(CFLAGS) -o $@ example.c ttpseng.o $(LFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)
