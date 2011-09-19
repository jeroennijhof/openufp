/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * n2h2.c: n2h2 frontend
 */

#include "openufp.h"

void n2h2_alive(int fd, struct n2h2_req *n2h2_request) {
    struct n2h2_resp n2h2_resp_alive;

    n2h2_resp_alive.code = htons(N2H2_ALIVE_RESP);
    n2h2_resp_alive.serial = n2h2_request->serial;
    n2h2_resp_alive.unknown = htons(0);
    n2h2_resp_alive.urlsize = htons(0);
 
    // send alive response
    send(fd, &n2h2_resp_alive, N2H2_HDR, 0);
}

void n2h2_accept(int fd, struct n2h2_req *n2h2_request) {
    struct n2h2_resp n2h2_resp_accept;

    n2h2_resp_accept.code = htons(N2H2_REQ_ACCEPT);
    n2h2_resp_accept.serial = n2h2_request->serial;
    n2h2_resp_accept.unknown = htons(0);
    n2h2_resp_accept.urlsize = htons(0);
 
    // send accept response
    send(fd, &n2h2_resp_accept, N2H2_HDR, 0);
}

void n2h2_deny(int fd, struct n2h2_req *n2h2_request, char *redirect_url) {
    struct n2h2_resp n2h2_resp_deny;
    int urlsize = 0;
    int i;

    n2h2_resp_deny.code = htons(N2H2_REQ_DENY);
    n2h2_resp_deny.serial = n2h2_request->serial;
    n2h2_resp_deny.unknown = htons(0);
    n2h2_resp_deny.urlsize = htons(0);

    // send custom redirect url if defined
    if (redirect_url != NULL) {
        urlsize = strlen(redirect_url) + 1;
        if (urlsize < URL_SIZE) {
            n2h2_resp_deny.urlsize = htons(urlsize);
            for(i = 0; i < urlsize; i++)
                n2h2_resp_deny.url[i] = redirect_url[i];
        }
    }

    // send denied response
    send(fd, &n2h2_resp_deny, N2H2_HDR + urlsize, 0);
}

struct uf_request n2h2_validate(struct n2h2_req *n2h2_request, int msgsize) {
    struct uf_request request = { 0, {0}, {0}, "" };
    struct in_addr srcip, dstip;
    int i;

    request.type = UNKNOWN;

    if (msgsize == N2H2_ALIVE_SIZE && ntohs(n2h2_request->code) == N2H2_ALIVE) {
        request.type = N2H2_ALIVE;
        return request;
    }

    if (msgsize > N2H2_REQ_SIZE && ntohs(n2h2_request->code) == N2H2_REQ && ntohs(n2h2_request->urlsize) < URL_SIZE) {
        request.type = N2H2_REQ;
        srcip.s_addr = n2h2_request->srcip;
        dstip.s_addr = n2h2_request->dstip;
        snprintf(request.srcip, sizeof(request.srcip), inet_ntoa(srcip));
        snprintf(request.dstip, sizeof(request.dstip), inet_ntoa(dstip));
        for(i = 0; i < ntohs(n2h2_request->urlsize); i++)
            request.url[i] = n2h2_request->url[i];
        return request;
    }

    return request;
}

