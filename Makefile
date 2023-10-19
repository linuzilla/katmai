CC = gcc

PREFIX=/usr/local

SUBDIRS = udplib tcplib ftpclient cron rs232 PDP_S6 playlist mactable \
	thread_svc misclib genpdu udpcmdrcv buffer_cache mcast \
	vod_table mini_filexchg md5sum simconf regexlib inetdlib \
	mqueue pqueue fmplib xtimer mp3lib conio ETen koklib \
	stream_mp3 mixer timeutil streamio_v2 streamio_v3 stio_engine xlist \
	katmai_version usec_timer stream_mpeg rmosdlib lyrics_osd holktv \
	hdllio idehdsn x_object db_mssql db_mysql bdb_hash sys_conf autofree

#	ttpseng hdllio 
#	rmosdlib


all:	libkatmai.a

libkatmai.a:
	rm -f libkatmai.a
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd $$subdir && $(MAKE))	\
	done;
	@ofiles=''; for ofile in `find . -name \*.o`; do \
		ofiles="$$ofiles $$ofile";\
	done; $(AR) rc libkatmai.a $$ofiles
	ranlib libkatmai.a
#		($(AR) rc libkatmai.a $$ofile);

example:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd $$subdir && $(MAKE) example)	\
	done;


incl-link:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd include; ln -sf ../$$subdir/*.h .) \
	done;

test:
	@for ifile in `find . -name \*.h`; do \
		(ln -sf .$$ifile include) \
	done;

clean:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		(cd $$subdir && $(MAKE) clean)	\
	done;
	rm -f libkatmai.a

install:	libkatmai.a
	[ -d $(PREFIX)/include/katmai ] || mkdir $(PREFIX)/include/katmai
	[ -d $(PREFIX)/lib/katmai ] || mkdir $(PREFIX)/lib/katmai
	install --mode=644 libkatmai.a $(PREFIX)/lib/katmai
	install --mode=644 include/* $(PREFIX)/include/katmai
