TARGET	= koklib.o
CC=gcc
CFLAGS+=-Wall -I../include
# -fpack-struct
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c koklib.o
	$(CC) $(CFLAGS) -o $@ example.c koklib.o -L../ -lkatmai

clean:
	rm -f $(TARGET)
