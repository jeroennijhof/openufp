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
#define WEBSNS_REQ 1
#define WEBSNS_REQ_SIZE 26

struct websns_req {
    u_int16_t size;
    u_int16_t vers_maj;
    u_int16_t vers_min;
    u_int16_t vers_pat;
    u_int32_t serial;
    u_int16_t code;
    u_int16_t desc;
    struct in_addr srcip;
    struct in_addr dstip;
    u_int16_t urlsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

struct websns_resp {
    u_int16_t size;
    u_int16_t vers_maj;
    u_int16_t vers_min;
    u_int16_t vers_pat;
    u_int32_t serial;
    u_int16_t code;
    u_int16_t desc;
    u_int16_t cat;
    u_int16_t urlsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

extern void websns_alive(int fd, struct websns_req *websns_request);
extern void websns_accept(int fd, struct websns_req *websns_request);
extern void websns_deny(int fd, struct websns_req *websns_request, char *redirect_url);
extern struct uf_request websns_validate(struct websns_req *websns_request, int msgsize);
extern struct websns_req *websns_convert(char msg[REQ_SIZE], int msgsize);

