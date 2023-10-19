include ../Environment.mak

TARGET	= snmp_lib.o snmpd_lib.o example

CC=gcc
# CFLAGS+=-Wall -I../include -g
#CFLAGS+=-D_REENTRANT -pthread
CCOPT   = -Wall -g -D_REENTRANT -pthread
INCLS   = -I../include
#CFLAGS+=-D_REENTRANT -pthread
INCLS   += -I$(NETSNMP_DIR)/include
# INCLS   += -I$(NETSNMP_DIR)/include/net-snmp
#DEFS   += -L$(NETSNMP_DIR)/lib/mysql
#DEFS   += -R$(FREETDS_DIR)/lib
LDIRS=-L$(NETSNMP_DIR)/lib
#LIBS=-lucdagent -lucdmibs -lsnmp -lz -ldl -lcrypto -lrpmdb
LIBS=-lnetsnmphelpers -lnetsnmpmibs -lnetsnmpagent -lnetsnmp -lcrypto
LIBS+=-lrpm-4.1 -lrpmdb-4.1 -lwrap

CFLAGS = $(CCOPT) $(INCLS) $(DEFS)

all:	$(TARGET)

example:	example.c snmp_lib.o snmpd_lib.o
	$(CC) $(CFLAGS) $(LDIRS) -o $@ example.c snmp_lib.o snmpd_lib.o $(LIBS)

clean:
	rm -f $(TARGET)

# gcc -shared -Wl,-soname,libx.so.1 -o libx.so.1.0.1 x_object.o -lc

