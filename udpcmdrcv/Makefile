TARGET	=	udpcmdrcv.o sendfile.o
CC=gcc
CFLAGS+=-Wall -I../include
CFLAGS+=-D_REENTRANT -pthread

all:	$(TARGET)

sftest:	sftest.c ../libkatmai.a
	gcc $(CFLAGS) -o $@ -L.. sftest.c -lkatmai

clean:
	rm -f $(TARGET)
