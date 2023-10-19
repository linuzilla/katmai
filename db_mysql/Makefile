include ../Environment.mak

TARGET	= db_mysql.o
# holktv_stb_cmd.o
CC	= gcc
CCOPT	= -Wall
INCLS	= -I../include
#CFLAGS+=-D_REENTRANT -pthread
INCLS	+= -I$(MYSQL_DIR)/include
DEFS   += -L$(MYSQL_DIR)/lib/mysql
#DEFS   += -R$(FREETDS_DIR)/lib

CFLAGS = $(CCOPT) $(INCLS) $(DEFS)

all:	$(TARGET)

example:	example.c db_mysql.o
	$(CC) $(CFLAGS) -o $@ example.c db_mysql.o -lmysqlclient -L.. -lkatmai

clean:
	rm -f $(TARGET)
