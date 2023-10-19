TARGET	= timeutil.o
CC=gcc
CFLAGS+=-Wall -I../include
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c ../libkatmai.a
	$(CC) $(CFLAGS) -o $@ example.c ../libkatmai.a

clean:
	rm -f $(TARGET)
