/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * proxy.c: proxy backend
 */

#include "openufp.h"

int proxy_backend(char *proxy_ip, int proxy_port, char *proxy_deny_pattern, char url[URL], int debug) {
    int proxy_fd = -1;
    int proxy_req_len = 0;
    int nbytes = 0;
    struct sockaddr_in proxy_addr;
    char proxy_res[PRXYRES];
    char proxy_req[URL+17];

    // return accept if socket failes
    if ((proxy_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        syslog(LOG_WARNING, "proxy: socket creation failed.");
        return 0;
    }

    bzero(&proxy_addr, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = inet_addr(proxy_ip);
    proxy_addr.sin_port = htons(proxy_port);

    // return accept if connect failes
    if (connect(proxy_fd, (struct sockaddr *) &proxy_addr, sizeof(proxy_addr)) < 0) {
        syslog(LOG_WARNING, "proxy: connecting to proxy failed.");
        close(proxy_fd);
        return 0;
    }

    // create proxy request
    bzero(proxy_req, sizeof(proxy_req));
    sprintf(proxy_req,"%s%s%s", "GET ", url, " HTTP/1.0\r\n\r\n");
    proxy_req_len = strlen(proxy_req);

    // Send the request to the proxy server, return accept if failes
    if (send(proxy_fd, proxy_req, proxy_req_len, 0) != proxy_req_len) {
        syslog(LOG_WARNING, "proxy: error while sending request to proxy.");
        close(proxy_fd);
        return 0;
    }

    // Recieve the proxy results
    bzero(proxy_res, sizeof(proxy_res));
    if ((nbytes = recv(proxy_fd, proxy_res, PRXYRES, MSG_WAITALL)) < 1) {
        syslog(LOG_WARNING, "proxy: error while receiving response from proxy.");
        close(proxy_fd);
        return 0;
    }

    close(proxy_fd);
 
    // check if the proxy is denying the page by the matched string
    proxy_res[nbytes] = '\0';
    if (debug > 2)
        syslog(LOG_INFO, "proxy: result (%s).", proxy_res);
    if((strstr(proxy_res, proxy_deny_pattern)) != NULL) {
       if (debug > 1)
           syslog(LOG_INFO, "proxy: url blocked.");
       return 1;
    }

    return 0;
}

