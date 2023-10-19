TARGET	= holktv_stb.o
# holktv_stb_cmd.o
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

example:	example.c holktv_stb.o
	$(CC) $(CFLAGS) -o $@ example.c holktv_stb.o \
			-L../ -L../../EM84XX/lib -lkatmai -lEM84xx -losd

example2:	example2.c holktv_stb.o
	$(CC) $(CFLAGS) -o $@ example2.c holktv_stb.o \
			-L../ -L../../EM84XX/lib -lkatmai -lEM84xx -losd
clean:
	rm -f $(TARGET) example
