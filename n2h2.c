/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * n2h2.c: n2h2 frontend
 */

#include "openufp.h"

void n2h2_alive(int fd, struct sockaddr_in cli_addr, char req_id[REQID]) {
    char mesg_alive[N2H2RES];
    int i;

    mesg_alive[0] = 3;
    mesg_alive[1] = 2;
    for(i = 0; i < 4; i++)
        mesg_alive[2+i] = req_id[i];
    for(i = 0; i < 4; i++)
        mesg_alive[6+i] = 0;
 
    // send alive response
    sendto(fd, mesg_alive, N2H2RES, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

void n2h2_accept(int fd, struct sockaddr_in cli_addr, char req_id[REQID]) {
    char mesg_accept[N2H2RES];
    int i;

    mesg_accept[0] = 0;
    mesg_accept[1] = 2;
    for(i = 0; i < 4; i++)
        mesg_accept[2+i] = req_id[i];
    for(i = 0; i < 4; i++)
        mesg_accept[6+i] = 0;

    // send accept response
    sendto(fd, mesg_accept, N2H2RES, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

void n2h2_deny(int fd, struct sockaddr_in cli_addr, char req_id[REQID], char *redirect_url) {
    char mesg_denied[N2H2RES+URL];
    int redirect_url_len = 0;
    int i;

    mesg_denied[0] = 1;
    mesg_denied[1] = 2;
    for(i = 0; i < 4; i++)
        mesg_denied[2+i] = req_id[i];
    if (redirect_url != NULL) {
        redirect_url_len = strlen(redirect_url) + 1;
        if (redirect_url_len > URL) {
            redirect_url_len = 0;
            for(i = 0; i < 4; i++)
                mesg_denied[6+i] = 0;
        } else {
            mesg_denied[6] = redirect_url_len / 768;
            mesg_denied[7] = (redirect_url_len % 768) / 512;
            mesg_denied[8] = ((redirect_url_len % 768) % 512) / 256;
            mesg_denied[9] = ((redirect_url_len % 768) % 512) % 256;
            for(i = 0; i < redirect_url_len; i++)
                mesg_denied[N2H2RES+i] = redirect_url[i];
        }
    } else {
        for(i = 0; i < 4; i++)
            mesg_denied[6+i] = 0;
    }

    // send denied response
    sendto(fd, mesg_denied, N2H2RES + redirect_url_len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

struct uf_request n2h2_request(char mesg[REQ]) {
    // URL Request req(2),reqid(4),srcip(4),dstip(4),urllen(2),usrlen(2),url(urllen),user(usrlen)
    struct uf_request request;
    int ips[8];
    int i;

    // Get type of request
    if ((mesg[0] == 2) && (mesg[1] == 3)) {
        request.type = N2H2ALIVE;
    }
    if ((mesg[0] == 2) && (mesg[1] == 0)) {
        request.type = N2H2REQ;
    }

    // Get request id
    for(i = 0; i < 4; i++)
        request.id[i] = mesg[2+i];

    // fetch srcip and dstip
    for(i = 0; i < 8; i++) {
        ips[i] = mesg[6+i];
        if (ips[i] < 0)
            ips[i] += 256;
    }
    bzero(request.srcip, sizeof(request.srcip));
    bzero(request.dstip, sizeof(request.dstip));
    sprintf(request.srcip, "%d.%d.%d.%d", ips[0], ips[1], ips[2], ips[3]);
    sprintf(request.dstip, "%d.%d.%d.%d", ips[4], ips[5], ips[6], ips[7]);

    // fetch url length
    request.urllen = (mesg[14]*256) + mesg[15];
    if (request.urllen < 0)
        request.urllen += 256;
    if (request.urllen > URL)
        request.urllen = URL;

    // fetch user length
    request.usrlen = (mesg[16]*256) + mesg[17];
    if (request.usrlen < 0)
        request.usrlen += 256;
    if (request.usrlen > USER)
        request.usrlen = USER;

    // fetch url
    for(i = 0; i < request.urllen; i++)
        request.url[i] = mesg[18+i];

    // fetch user
    for(i = 0; i < request.usrlen; i++)
        request.user[i] = mesg[18+request.urllen+i];

    return request;
}

