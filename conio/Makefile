TARGET	= conio.o
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

testing:	testing.c conio.o
	$(CC) $(CFLAGS) -o $@ testing.c conio.o

clean:
	rm -f $(TARGET)
