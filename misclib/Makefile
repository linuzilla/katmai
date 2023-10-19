TARGET	=	rtrim.o ltrim.o chomp.o strip_dos_eof.o fork_exec.o \
		local_addr.o timer_util.o \
		md5.o rc4encrypt.o lprintf.o
CC	=	gcc
CFLAGS  =	-Wall -O2

all:	$(TARGET)

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm -f $(TARGET)
