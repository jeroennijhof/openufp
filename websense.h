/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * websense.h: websense frontend
 */

#define WEBSNS 2
#define WEBSNS_HDR 20
#define WEBSNS_ALIVE 12
#define WEBSNS_ALIVE_RESP 0
#define WEBSNS_ALIVE_SIZE 12
#define WEBSNS_REQ 1
#define WEBSNS_REQ_ACCEPT 0
#define WEBSNS_REQ_DENY 1
#define WEBSNS_REQ_SIZE 26

struct websns_req {
    uint16_t size;
    uint16_t vers_maj;
    uint16_t vers_min;
    uint16_t vers_pat;
    uint32_t serial;
    uint16_t code;
    uint16_t desc;
    uint32_t srcip;
    uint32_t dstip;
    uint16_t urlsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

struct websns_resp {
    uint16_t size;
    uint16_t vers_maj;
    uint16_t vers_min;
    uint16_t vers_pat;
    uint32_t serial;
    uint16_t code;
    uint16_t desc;
    uint16_t cat;
    uint16_t urlsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

extern void websns_alive(int fd, struct websns_req *websns_request);
extern void websns_accept(int fd, struct websns_req *websns_request);
extern void websns_deny(int fd, struct websns_req *websns_request, char *redirect_url);
extern struct uf_request websns_validate(struct websns_req *websns_request, int msgsize);
extern void websns_convert(struct websns_req *websns_request, char msg[REQ_SIZE], int msgsize, int debug);

