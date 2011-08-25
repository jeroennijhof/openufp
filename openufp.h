#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>

#define VERSION "1.05"
#define URL 65536
#define USER 512
#define REQ 26+URL+USER
#define REQID 10
#define IP 16

// request struct
struct uf_request {
    char id[REQID];
    int type;
    char srcip[IP];
    char dstip[IP];
    int urllen;
    char url[URL];
    int usrlen;
    char user[USER];
};

// cache
#include "cache.h"

// frontends
#include "n2h2.h"
#include "websense.h"

// backends
#include "blacklist.h"
#include "proxy.h"
#include "squidguard.h"

