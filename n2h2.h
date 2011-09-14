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
#define N2H2_REQ 512
#define N2H2_REQ_SIZE 18

struct n2h2_req {
    u_int16_t code;
    u_int32_t serial;
    struct in_addr srcip;
    struct in_addr dstip;
    u_int16_t urlsize;
    u_int16_t usrsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

struct n2h2_resp {
    u_int16_t code;
    u_int32_t serial;
    u_int16_t unknown;
    u_int16_t urlsize;
    char url[URL_SIZE];
} __attribute__((__packed__));

extern void n2h2_alive(int fd, struct n2h2_req *n2h2_request);
extern void n2h2_accept(int fd, struct n2h2_req *n2h2_request);
extern void n2h2_deny(int fd, struct n2h2_req *n2h2_request, char *redirect_url);
extern struct uf_request n2h2_validate(struct n2h2_req *n2h2_request, int msgsize);

