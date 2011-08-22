/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * n2h2.h: n2h2 frontend
 */

#define N2H2 1
#define N2H2REQ 3
#define N2H2ALIVE 5
#define N2H2RES 10

extern void n2h2_alive(int fd, struct sockaddr_in cli_addr, char req_id[REQID]);
extern void n2h2_accept(int fd, struct sockaddr_in cli_addr, char req_id[REQID]);
extern void n2h2_deny(int fd, struct sockaddr_in cli_addr, char req_id[REQID], char *redirect_url);
extern struct uf_request n2h2_request(char mesg[REQ]);

