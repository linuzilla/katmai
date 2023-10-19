TARGET	=	cron.o cronjob.o
# TARGET	=	cron.o

all:	$(TARGET)

cron.o:	cron.c
	gcc -Wall -O2 -pthread -c $<

cronjob.o:	cronjob.c
	gcc -Wall -O2 -pthread -c $<

crontest:	cron.o cronjob.o
	gcc -Wall -pthread -o $@ $(TARGET)

clean:
	rm -f $(TARGET)
