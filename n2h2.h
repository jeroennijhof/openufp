/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * n2h2.h: n2h2 frontend
 */

#define N2H2 1
#define N2H2_HDR 10
#define N2H2_ALIVE 515
#define N2H2_ALIVE_RESP 770
#define N2H2_ALIVE_SIZE 20
#define N2H2_REQ 512
#define N2H2_REQ_ACCEPT 2
#define N2H2_REQ_DENY 258
#define N2H2_REQ_SIZE 18

struct n2h2_req {
    uint16_t code;
    uint32_t serial;
    uint32_t srcip;
    uint32_t dstip;
    uint16_t urlsize;
    uint16_t usrsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

struct n2h2_resp {
    uint16_t code;
    uint32_t serial;
    uint16_t unknown;
    uint16_t urlsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

extern void n2h2_alive(int fd, struct n2h2_req *n2h2_request);
extern void n2h2_accept(int fd, struct n2h2_req *n2h2_request);
extern void n2h2_deny(int fd, struct n2h2_req *n2h2_request, char *redirect_url);
extern struct uf_request n2h2_validate(struct n2h2_req *n2h2_request, int msgsize);

