TARGET	= rmosdlib.o rmimage256.o
CC=gcc
CFLAGS+=-Wall -I./include -I./inc -I../include -g
CFLAGS+=-D_BARBADOS_
CFLAGS+=-L..
#CFLAGS+=-L../../EM84XX/lib -R../../EM84XX/lib
# CFLAGS+=-D_REENTRANT -pthread

OBJECT=	osd.o rmosdlib.o rmimage256.o

all:	$(TARGET) rgb2cyu

example:	$(OBJECT)
	$(CC) $(CFLAGS) -o $@ $(OBJECT) -lEM84xx -losd -lkatmai

rgb2cyu:	rgb2cyu.c
	$(CC) $(CFLAGS) -o $@ rgb2cyu.c

clean:
	rm -f $(TARGET) $(OBJECT) example
