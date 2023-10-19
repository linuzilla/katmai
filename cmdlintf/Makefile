TARGET	= cmdlintf.o example

CC=gcc
CFLAGS+=-Wall -I../include -g
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c cmdlintf.o
	$(CC) $(CFLAGS) -o $@ example.c cmdlintf.o -lreadline -ltermcap

clean:
	rm -f $(TARGET)

# gcc -shared -Wl,-soname,libx.so.1 -o libx.so.1.0.1 x_object.o -lc
