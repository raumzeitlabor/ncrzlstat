CC= 		gcc
CXX= 		g++
CFLAGS+= 	-I/usr/include
CFLAGS+=	-std=c99
CFLAGS+=	-g
LDFLAGS+=	-L/usr/lib
LDADD+=		-lcurl
LDADD+=		-ljansson
LDADD+=		-lncurses

BIN=	ncrzlstat
OBJ+=	ncrzlstat.o
OBJ+=	fetch.o
OBJ+=	parse.o
OBJ+=	display.o

all:	$(BIN)

$(BIN):	$(OBJ)
	$(CC) $(LDFLAGS) $(LDADD) -o $@ $(OBJ)

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(BIN)
