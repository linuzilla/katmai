include ../Environment.mak

TARGET	= db_mssql.o
# holktv_stb_cmd.o
CC	= gcc
CCOPT	= -Wall
INCLS	= -I../include
#CFLAGS+=-D_REENTRANT -pthread
INCLS	+= -I$(FREETDS_DIR)/include
#DEFS   += -L$(FREETDS_DIR)/lib
#DEFS   += -R$(FREETDS_DIR)/lib

CFLAGS = $(CCOPT) $(INCLS) $(DEFS)

all:	$(TARGET)

clean:
	rm -f $(TARGET)
