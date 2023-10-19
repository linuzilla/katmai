TARGET	= tcplib.o testing
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

testing:	testing.c tcplib.o
	$(CC) $(CFLAGS) -o $@ testing.c tcplib.o

clean:
	rm -f $(TARGET)
