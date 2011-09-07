/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * websense.h: websense frontend
 */

#define WEBSNS 2
#define WEBSNSREQ 4
#define WEBSNSALIVE 6
#define WEBSNSRES 34

extern void websns_accept(int fd, struct sockaddr_in cli_addr, char req_id[REQID]);
extern void websns_deny(int fd, struct sockaddr_in cli_addr, char req_id[REQID], char *redirect_url);
extern struct uf_request websns_request(char mesg[REQ]);

