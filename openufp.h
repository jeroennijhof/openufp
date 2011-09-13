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

#define VERSION "1.06"
#define URL_SIZE 65535
#define REQ_SIZE 65535

// request struct and defs
#define UNKNOWN 65535
struct uf_request {
    u_int16_t type;
    struct in_addr srcip;
    struct in_addr dstip;
    char url[URL_SIZE];
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

