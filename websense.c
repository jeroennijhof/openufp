/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * websense.c: websense frontend
 */

#include "openufp.h"

void websns_alive(int fd, struct websns_req *websns_request) {
    struct websns_resp websns_resp_alive;

    websns_resp_alive.size = htons(WEBSNS_HDR);
    websns_resp_alive.vers_maj = websns_request->vers_maj;
    websns_resp_alive.vers_min = websns_request->vers_min;
    websns_resp_alive.vers_pat = websns_request->vers_pat;
    websns_resp_alive.serial = websns_request->serial;
    websns_resp_alive.code = htons(WEBSNS_ALIVE_RESP);
    websns_resp_alive.desc = htons(65535);
    websns_resp_alive.cat = htons(0);
    websns_resp_alive.urlsize = htons(0);

    // send accept response
    send(fd, &websns_resp_alive, WEBSNS_HDR, 0);
}

void websns_accept(int fd, struct websns_req *websns_request) {
    struct websns_resp websns_resp_accept;

    websns_resp_accept.size = htons(WEBSNS_HDR);
    websns_resp_accept.vers_maj = websns_request->vers_maj;
    websns_resp_accept.vers_min = websns_request->vers_min;
    websns_resp_accept.vers_pat = websns_request->vers_pat;
    websns_resp_accept.serial = websns_request->serial;
    websns_resp_accept.code = htons(WEBSNS_REQ_ACCEPT);
    websns_resp_accept.desc = htons(0);
    websns_resp_accept.cat = htons(0);
    websns_resp_accept.urlsize = htons(0);

    // send accept response
    send(fd, &websns_resp_accept, WEBSNS_HDR, 0);
}

void websns_deny(int fd, struct websns_req *websns_request, char *redirect_url) {
    struct websns_resp websns_resp_deny;
    int urlsize = 0;
    int i = 0;

    websns_resp_deny.size = htons(WEBSNS_HDR);
    websns_resp_deny.vers_maj = websns_request->vers_maj;
    websns_resp_deny.vers_min = websns_request->vers_min;
    websns_resp_deny.vers_pat = websns_request->vers_pat;
    websns_resp_deny.serial = websns_request->serial;
    websns_resp_deny.code = htons(WEBSNS_REQ_DENY);
    websns_resp_deny.desc = htons(1);
    websns_resp_deny.cat = htons(0);
    websns_resp_deny.urlsize = htons(0);

    if (redirect_url != NULL) {
        urlsize = strlen(redirect_url) + 1;
        if (urlsize < (URL_SIZE - WEBSNS_HDR)) {
            websns_resp_deny.size = htons(WEBSNS_HDR + urlsize);
            websns_resp_deny.urlsize = htons(urlsize);
            for(i = 0; i < urlsize; i++)
                websns_resp_deny.url[i] = redirect_url[i];
        }
    }

    // send denied response
    send(fd, &websns_resp_deny, WEBSNS_HDR + urlsize, 0);
}

struct uf_request websns_validate(struct websns_req *websns_request, int msgsize) {
    struct uf_request request = { 0, {0}, {0}, "" };
    struct in_addr srcip, dstip;
    int i;

    request.type = UNKNOWN;

    if (msgsize == WEBSNS_ALIVE_SIZE) {
        request.type = WEBSNS_ALIVE;
        return request;
    }

    if (msgsize > WEBSNS_REQ_SIZE && ntohs(websns_request->code) == WEBSNS_REQ && ntohs(websns_request->urlsize) < URL_SIZE) {
        request.type = WEBSNS_REQ;
        srcip.s_addr = websns_request->srcip;
        dstip.s_addr = websns_request->dstip;
        snprintf(request.srcip, sizeof(request.srcip), "%s", inet_ntoa(srcip));
        snprintf(request.dstip, sizeof(request.dstip), "%s", inet_ntoa(dstip));
        for(i = 0; i < ntohs(websns_request->urlsize); i++)
            request.url[i] = websns_request->url[i];
        return request;
    }

    return request;
}

void websns_convert(struct websns_req *websns_request, char msg[REQ_SIZE], int msgsize) {
    char newmsg[REQ_SIZE];
    int offset = 0;
    int i;

    // check if it's version 1
    if (msgsize > WEBSNS_REQ_SIZE && ntohs(websns_request->code) == WEBSNS_REQ && ntohs(websns_request->urlsize) == 0) {
        // convert to version 4
        for (i = 0; i < (msgsize - 2); i++) {
            if (i == 24)
                offset = 2;
            newmsg[i] = msg[i + offset];
        }
        struct websns_req *websns_vers1 = (struct websns_req *)newmsg;
        *websns_request = *websns_vers1;
    }
}
