# TARGET	=	ftpclient ftpclient.o
TARGET	=	udplib.o

all:	$(TARGET)

udplib.o:	udplib.c
	gcc -Wall -O2 -c $<

clean:
	rm -f $(TARGET)
