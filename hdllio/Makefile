TARGET	= hdllio.o example
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c hdllio.o
	$(CC) $(CFLAGS) -o $@ example.c hdllio.o
#-lext2fs

clean:
	rm -f $(TARGET)
