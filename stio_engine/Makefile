TARGET	= stio_engine.o
CC=gcc
CFLAGS+=-Wall -I../include
# -fpack-struct
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

stio_engine.o:	stio_engine.c prefetch.c
	$(CC) $(CFLAGS) -c -o $@ stio_engine.c

example:	example.c stio_engine.o
	$(CC) $(CFLAGS) -o $@ example.c stio_engine.o -L.. -lkatmai

clean:
	rm -f $(TARGET) example
