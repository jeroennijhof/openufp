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
#include <stdbool.h>
#include <regex.h>

#define VERSION "1.08"
#define URL_SIZE 65535
#define REQ_SIZE 65535

// request struct and defs
#define UNKNOWN 65535
struct uf_request {
    uint16_t type;
    char srcip[15];
    char dstip[15];
    char url[URL_SIZE];
    char usr[URL_SIZE];
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

