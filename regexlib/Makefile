TARGET	= regexlib.o
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c regexlib.o
	$(CC) $(CFLAGS) -o $@ example.c regexlib.o

clean:
	rm -f $(TARGET)
