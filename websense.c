/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * websense.c: websense frontend
 */

#include "openufp.h"

void websns_alive(int fd, struct sockaddr_in cli_addr, char req_id[REQID]) {
    char mesg_accept[WEBSNSHDR];
    int i = 0;

    mesg_accept[0] = 0;
    mesg_accept[1] = WEBSNSHDR;
    for(i = 0; i < 10; i++)
        mesg_accept[2+i] = req_id[i];
    for(i = 0; i < 8; i++)
        mesg_accept[12+i] = 0;
    mesg_accept[14] = 255;
    mesg_accept[15] = 255;

    // send accept response
    sendto(fd, mesg_accept, WEBSNSHDR, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

void websns_accept(int fd, struct sockaddr_in cli_addr, char req_id[REQID]) {
    char mesg_accept[WEBSNSHDR];
    int i = 0;

    mesg_accept[0] = 0;
    mesg_accept[1] = WEBSNSHDR;
    for(i = 0; i < 10; i++)
        mesg_accept[2+i] = req_id[i];
    for(i = 0; i < 8; i++)
        mesg_accept[12+i] = 0;

    // send accept response
    sendto(fd, mesg_accept, WEBSNSHDR, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
}

void websns_deny(int fd, struct sockaddr_in cli_addr, char req_id[REQID], char *redirect_url) {
    char mesg_denied[WEBSNSHDR+URL];
    int redirect_url_len = 0;
    int i = 0;

    mesg_denied[0] = 0;
    mesg_denied[1] = WEBSNSHDR;
    for(i = 0; i < 10; i++)
        mesg_denied[2+i] = req_id[i];
    mesg_denied[12] = 0; // code
    mesg_denied[13] = 1; // code
    mesg_denied[14] = 0;  // desc
    mesg_denied[15] = 1; // desc
    mesg_denied[16] = 0;   // cat
    mesg_denied[17] = 0; // cat
    mesg_denied[18] = 0; // url_size
    mesg_denied[19] = 0; // url_size

    if (redirect_url != NULL) {
        redirect_url_len = strlen(redirect_url) + 1;
        if (redirect_url_len <= URL) {
            mesg_denied[1] += redirect_url_len;
            mesg_denied[19] = redirect_url_len;
            for(i = 0; i < redirect_url_len; i++)
                mesg_denied[WEBSNSHDR+i] = redirect_url[i];
        }
    }

    // send denied response
    sendto(fd, mesg_denied, WEBSNSHDR + redirect_url_len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
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

