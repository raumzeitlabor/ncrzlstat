CC= 		gcc
CXX= 		g++
CFLAGS+= 	-I/usr/include
CFLAGS+=	-std=c99
CFLAGS+=	-g
LDFLAGS+=	-L/usr/lib
LDADD+=		-lcurl
LDADD+=		-ljansson

ifeq ($(shell uname -s),SunOS)
	CFLAGS+=	-I/opt/csw/include
	LDFLAGS+=	-L/opt/csw/lib -R/opt/csw/lib
	LDADD+=		-lcurses
else
	LDADD+=		-lncurses
endif

BIN=	ncrzlstat
OBJ+=	ncrzlstat.o
OBJ+=	fetch.o
OBJ+=	parse.o
OBJ+=	ui.o

all:	$(BIN)

$(BIN):	$(OBJ)
	$(CC) $(LDFLAGS) $(LDADD) -o $@ $(OBJ)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(BIN)
