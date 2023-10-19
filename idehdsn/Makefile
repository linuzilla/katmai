TARGET	= idehdsn.o example
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c idehdsn.o
	$(CC) $(CFLAGS) -o $@ example.c idehdsn.o

clean:
	rm -f $(TARGET)
