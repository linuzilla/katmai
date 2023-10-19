# TARGET	=	ftpclient ftpclient.o
TARGET	=	ftpclient.o

all:	$(TARGET)

ftpclient.o:	ftpclient.c
	gcc -Wall -O2 -c $<

ftpclient:	ftpclient.o
	gcc -Wall -o $@ $<

clean:
	rm -f $(TARGET)
