TARGET	=	buffer_cache.o

all:	$(TARGET)

buffer_cache.o:	buffer_cache.c
	gcc -Wall -pthread -O2 -g -c $<

clean:
	rm -f $(TARGET)
