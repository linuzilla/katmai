TARGET	= lyrics_osd.o
CC=gcc
CFLAGS+=-Wall -I../include
# -fpack-struct
CFLAGS+=-D_REENTRANT -pthread
CFLAGS+=-D_BARBADOS_
#CFLAGS+=-L../
#CFLAGS+=-L../../EM84XX/lib -R../../EM84XX/lib

all:	$(TARGET)

example:	example.c lyrics_osd.o
	$(CC) $(CFLAGS) -o $@ example.c lyrics_osd.o \
		-L../ -L../../EM84XX/lib -lkatmai -lEM84xx -losd

clean:
	rm -f $(TARGET) example
