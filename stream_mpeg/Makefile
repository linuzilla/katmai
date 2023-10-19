TARGET	= stream_mpeg.o
CC=gcc
CFLAGS+=-Wall -I../include
# -fpack-struct
CFLAGS+=-D_REENTRANT -pthread
CFLAGS+=-D_BARBADOS_
#CFLAGS+=-L../
#CFLAGS+=-L../../EM84XX/lib -R../../EM84XX/lib

all:	$(TARGET)

stream_mpeg.o:	stream_mpeg.c play_public.c
	$(CC) $(CFLAGS) -c -o $@ $<

example:	example.c stream_mpeg.o
	$(CC) $(CFLAGS) -o $@ example.c stream_mpeg.o \
		-L../ -L../../EM84XX/lib -lkatmai -lEM84xx -losd

clean:
	rm -f $(TARGET) example
