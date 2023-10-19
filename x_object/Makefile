TARGET	= x_object.o

CC=gcc
CFLAGS+=-Wall -I../include -g
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c x_object.o
	$(CC) $(CFLAGS) -o $@ example.c x_object.o -L.. -lkatmai

clean:
	rm -f $(TARGET)

# gcc -shared -Wl,-soname,libx.so.1 -o libx.so.1.0.1 x_object.o -lc
