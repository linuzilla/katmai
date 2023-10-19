#TARGET	= server.o client.o client_entry.o
TARGET	= server.o stb_table.o client.o
CC=gcc
CFLAGS+=-Wall -I./ -I../include -I./include -g
CFLAGS+=-D_REENTRANT -pthread
CFLAGS+=-finline-functions -funroll-all-loops
#CFLAGS+=-fomit-frame-pointer -funroll-all-loops
#CFLAGS+=-finline-functions -ffast-math

all:	$(TARGET)

example:	example.c $(TARGET)
	$(CC) $(CFLAGS) -o $@ example.c $(TARGET) -L../ -lkatmai

example2:	example2.c $(TARGET)
	$(CC) $(CFLAGS) -o $@ example2.c $(TARGET) -L../ -lkatmai

clean:
	rm -f $(TARGET) example
