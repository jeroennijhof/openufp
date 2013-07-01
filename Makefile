# make the openufp server
#
OWNER=root
GROUP=root
CC=gcc
CFLAGS=-O2 -Wall
LIBS=-ldb
SBINDIR=/usr/sbin

all:	openufp

openufp:
		$(CC) $(CFLAGS) openufp.c cache.c n2h2.c websense.c blacklist.c proxy.c squidguard.c -o $@ $(LIBS)

install:	openufp
		install -o $(OWNER) -g $(GROUP) -m 755 openufp $(SBINDIR)
		install -o $(OWNER) -g $(GROUP) -m 755 -d /var/cache/openufp

clean:
		rm -f openufp
