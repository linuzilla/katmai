TARGET	= version.o
CC=gcc
CFLAGS+=-Wall -I../include
VER := $(shell sed -e 's/.*\"\(.*\)\"/\1/' ../VERSION)
CFLAGS += -DVERSION=\"$(VER)\"
#CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

clean:
	rm -f $(TARGET) example
