/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * websense.c: websense frontend
 */

#include "openufp.h"

void websns_accept(int fd, struct sockaddr_in cli_addr, char req_id[REQID]) {
    // reqsize(2),reqid(10),code(2),desc(2),category(2),cache?(4),cachecmd(2),cachetype(2),null(8)
    char mesg_accept[WEBSNSRES];
    int i = 0;

    mesg_accept[0] = 0;
    mesg_accept[1] = WEBSNSRES;
    for(i = 0; i < 10; i++)
        mesg_accept[2+i] = req_id[i];
    mesg_accept[12] = 0;
    mesg_accept[13] = 0;
    mesg_accept[14] = 4;
    mesg_accept[15] = 10;
    mesg_accept[16] = 0;
    mesg_accept[17] = 153;
    for(i = 0; i < 16; i++)
        mesg_accept[18+i] = 0;

    // send accept response
    sendto(fd, mesg_accept, WEBSNSRES, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

void websns_deny(int fd, struct sockaddr_in cli_addr, char req_id[REQID], char *redirect_url) {
    // reqsize(2),reqid(10),code(2),desc(2),category(2),cache?(4),cachecmd(2),cachetype(2),null(8)
    char mesg_denied[WEBSNSRES+URL];
    int redirect_url_len = 0;
    int i = 0;

    mesg_denied[0] = 0;
    mesg_denied[1] = WEBSNSRES;
    for(i = 0; i < 10; i++)
        mesg_denied[2+i] = req_id[i];
    mesg_denied[12] = 0; // code
    mesg_denied[13] = 1; // code
    mesg_denied[14] = 4;  // desc
    mesg_denied[15] = 10; // desc
    mesg_denied[16] = 0;   // cat
    mesg_denied[17] = 153; // cat
    for(i = 0; i < 16; i++)
        mesg_denied[18+i] = 0;

    // send custom redirect url if defined
    // not working yet so disabled
    redirect_url = NULL;
    if (redirect_url != NULL) {
        redirect_url_len = strlen(redirect_url) + 1;
        if (redirect_url_len <= URL) {
            mesg_denied[30] = redirect_url_len / 768;
            mesg_denied[31] = (redirect_url_len % 768) / 512;
            mesg_denied[32] = ((redirect_url_len % 768) % 512) / 256;
            mesg_denied[33] = ((redirect_url_len % 768) % 512) % 256;
            for(i = 0; i < redirect_url_len; i++)
                mesg_denied[N2H2RES+i] = redirect_url[i];
        }
    }

    // send denied response
    sendto(fd, mesg_denied, WEBSNSRES, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

struct uf_request websns_request(char mesg[REQ]) {
    /* version 1:
       reqsize(2),reqid(10),code(2),descr(2),srcip(4),dstip(4),usrsize(2),urlsize(2),url(urlsize)
       version 4:
       reqsize(2),reqid(10),code(2),descr(2),srcip(4),dstip(4),urlsize(2),url(urlsize) */
    struct uf_request request = {"", 0, "", "", 0, "", 0, ""};
    int ips[8];
    int i = 0;
    int offset = 0;

    // Get request id
    for(i = 0; i < 10; i++)
        request.id[i] = mesg[2+i];

    // Get type of request
    if ((mesg[0] == 0) && (mesg[1] == 12)) {
        request.type = WEBSNSALIVE;
        return request;
    }
    if ((mesg[12] == 0) && (mesg[13] == 1)) {
        request.type = WEBSNSREQ;
    }

    // fetch srcip and dstip
    for(i = 0; i < 8; i++) {
        ips[i] = mesg[16+i];
        if (ips[i] < 0)
            ips[i] += 256;
    }
    bzero(request.srcip, sizeof(request.srcip));
    bzero(request.dstip, sizeof(request.dstip));
    sprintf(request.srcip,"%d.%d.%d.%d", ips[0], ips[1], ips[2], ips[3]);
    sprintf(request.dstip,"%d.%d.%d.%d", ips[4], ips[5], ips[6], ips[7]);

    // check version
    if (mesg[24] == 0 && mesg[25] == 0)
        offset = 2;

    // fetch url length
    request.urllen = (mesg[24+offset]*256) + mesg[25+offset];
    if (request.urllen < 0)
        request.urllen += 256;
    if (request.urllen > URL)
        request.urllen = URL;

    // fetch url
    for(i = 0; i < request.urllen; i++)
        request.url[i] = mesg[26+offset+i];

    return request;
}

