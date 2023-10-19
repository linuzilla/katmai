include ../Environment.mak

TARGET	= ssl_lib.o example POP3-Agent

CC=gcc
# CFLAGS+=-Wall -I../include -g
#CFLAGS+=-D_REENTRANT -pthread
CCOPT   = -Wall -g
INCLS   = -I../include
#CFLAGS+=-D_REENTRANT -pthread
# INCLS   += -I$(OPENSSL_DIR)/include/openssl
# INCLS   += -I$(NETSNMP_DIR)/include/net-snmp
#DEFS   += -L$(NETSNMP_DIR)/lib/mysql
#DEFS   += -R$(FREETDS_DIR)/lib
#LDIRS=-L$(NETSNMP_DIR)/lib
#LIBS=-lucdagent -lucdmibs -lsnmp -lz -ldl -lcrypto -lrpmdb
#LIBS=-lnetsnmphelpers -lnetsnmpmibs -lnetsnmpagent -lnetsnmp -lcrypto
LIBS=-lssl
#LIBS+=-lrpm-4.1 -lrpmdb-4.1 -lwrap

CFLAGS = $(CCOPT) $(INCLS) $(DEFS)

all:	$(TARGET)

example:	example.c ssl_lib.o
	$(CC) $(CFLAGS) $(LDIRS) -o $@ example.c ssl_lib.o $(LIBS)

POP3-Agent:	POP3-Agent.c ssl_lib.o
	$(CC) $(CFLAGS) $(LDIRS) -o $@ POP3-Agent.c ssl_lib.o $(LIBS)

clean:
	rm -f $(TARGET)

# gcc -shared -Wl,-soname,libx.so.1 -o libx.so.1.0.1 x_object.o -lc

