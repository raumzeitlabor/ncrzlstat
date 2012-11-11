PROG=		ncrzlstat

CFLAGS+=	-I/usr/local/include
LDFLAGS+=	-L/usr/local/lib
LDADD+=		-lcurl
LDADD+=		-ljansson
LDADD+=		-lncurses

WARNS?=		6

NO_MAN=		YES

.include <bsd.prog.mk>
