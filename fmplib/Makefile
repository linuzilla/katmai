TARGET	= fmplib.o
CC=gcc
CFLAGS+=-Wall -I../include -I../SUPPORT/EM84XX/include
CFLAGS+=-D_BARBADOS_
CFLAGS+=-L..
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:        example.c fmplib.o
	$(CC) $(CFLAGS) -o $@ example.c fmplib.o \
		-L../ -L../SUPPORT/EM84XX/lib -lkatmai -lEM84xx -losd

clean:
	rm -f $(TARGET)
