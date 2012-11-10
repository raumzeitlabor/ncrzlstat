CC= 		gcc
CXX= 		g++
CFLAGS+= 	-I/usr/include
CFLAGS+=	-std=c99
CFLAGS+=	-g
LDFLAGS+=	-L/usr/lib
LDADD+=		-lcurl
LDADD+=		-ljansson
LDADD+=		-lncurses

all:	ncrzlstat

ncrzlstat.o:	ncrzlstat.c
	$(CC) $(CFLAGS) -c ncrzlstat.c

ncrzlstat:	ncrzlstat.o
	$(CC) $(LDFLAGS) $(LDADD) -o ncrzlstat ncrzlstat.o
clean:
	rm *.o ncrzlstat
