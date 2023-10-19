TARGET	= stream_mp3.o
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread
# -fpack-struct

all:	$(TARGET)

example:	example.c stream_mp3.o
	$(CC) $(CFLAGS) -o $@ example.c stream_mp3.o -L../ -lkatmai -lm

clean:
	rm -f $(TARGET) example
