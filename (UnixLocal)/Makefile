CC      = cc
MYLIB   = ../libnet.a
LIBS    =
CFLAGS  = -O -g -fullwarn

*.o: gcc -O $@.c

tcpserv:	tcpserv.o strecho.o
	        $(CC) $(CFLAGS) -o $@ tcpserv.o strecho.o $(MYLIB) $(LIBS)

unixservmain.o unixlocalutils.o localservutils.o: s-server.h
unixprefs.o unixinterface.o unixtcp.o maccompat.o: s-server.h
m-assets.o s-rooms.o s-assets.o s-script.o s-author.o: s-server.h
m-utils.o s-evernts.o s-users.o: s-server.h
s-file.o s-secure.o s-usrbas.o: s-server.h

pserver: unixservmain.o unixlocalutils.o localservutils.o unixprefs.o \
m-assets.o s-rooms.o s-assets.o s-script.o s-author.o m-utils.o \
s-events.o s-users.o s-file.o s-secure.o s-usrbas.o \
unixinterface.o unixtcp.o maccompat.o semaph.o
	$(CC) $(CFLAGS) -o $@ unixservmain.o unixlocalutils.o localservutils.o \
 unixprefs.o m-assets.o s-rooms.o s-assets.o s-script.o s-author.o \
 m-utils.o s-events.o s-users.o s-file.o s-secure.o s-usrbas.o \
 unixinterface.o unixtcp.o maccompat.o semaph.o


