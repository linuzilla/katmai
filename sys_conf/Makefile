include ../Environment.mak

TARGET	= sys_conf.o lex.yy.o y.tab.o
# holktv_stb_cmd.o
CC	= gcc
CCOPT	= -Wall
INCLS	= -I../include -I./include
#CFLAGS+=-D_REENTRANT -pthread
#DEFS   += -L$(FREETDS_DIR)/lib
#DEFS   += -R$(FREETDS_DIR)/lib
LEXYACCTMP = lex.yy.c y.tab.c y.tab.h y.output y.tab.o lex.yy.o

CFLAGS = $(CCOPT) $(INCLS) $(DEFS)

all:	$(TARGET)

y.tab.c:        parser.y
	bison -v -t -d -y parser.y

lex.yy.c:       lexer.l y.tab.c
	flex lexer.l

clean:
	rm -f $(TARGET) $(LEXYACCTMP)
