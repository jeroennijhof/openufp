# make the openufp server
#
OWNER=root
GROUP=root
CC=gcc
CFLAGS=-O2 -Wall
SBINDIR=/usr/sbin

all:	openufp

openufp:
		$(CC) $(CFLAGS) openufp.c n2h2.c websense.c proxy.c -o $@

install:	openufp
		install -c -o $(OWNER) -g $(GROUP) -m 755 openufp $(SBINDIR)

clean:
		rm -f openufp
