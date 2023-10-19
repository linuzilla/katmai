include ../Environment.mak

TARGET	= bdb_hash.o example
# holktv_stb_cmd.o
CC	= gcc
CCOPT	= -Wall
INCLS	= -I../include
#CFLAGS+=-D_REENTRANT -pthread
INCLS	+= -I$(BERKELEY_DB_DIR)/include
#DEFS   += -L$(FREETDS_DIR)/lib
#DEFS   += -R$(FREETDS_DIR)/lib

CFLAGS = $(CCOPT) $(INCLS) $(DEFS)

all:	$(TARGET)

example:	example.c
	$(CC) $(CFLAGS) -o $@ example.c bdb_hash.o -ldb

clean:
	rm -f $(TARGET)
