/* openufp server
 *
 * author: Jeroen Nijhof
 * version: 1.04
 * license: GPL v3.0
 *
 * proxy.c: proxy backend
 */

#include "openufp.h"

int proxy_backend(char *proxy_ip, int proxy_port, char *proxy_deny_pattern, char proxy_url[URL]) {
    int proxy_fd;
    int proxy_req_len;
    int nbytes;
    struct sockaddr_in proxy_addr;
    char proxy_res[PRXYRES];
    char proxy_req[URL+17];

    return 1;

    // return accept if socket failes
    if ((proxy_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        syslog(LOG_WARNING, "proxy socket creation failed.");
        return 0;
    }

    bzero(&proxy_addr, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = inet_addr(proxy_ip);
    proxy_addr.sin_port = htons(proxy_port);

    // return accept if connect failes
    if (connect(proxy_fd, (struct sockaddr *) &proxy_addr, sizeof(proxy_addr)) < 0) {
        syslog(LOG_WARNING, "connecting to proxy failed.");
        close(proxy_fd);
        return 0;
    }

    // create proxy request
    bzero(proxy_req, sizeof(proxy_req));
    sprintf(proxy_req,"%s%s%s\r\n\r\n", "GET ", proxy_url, " HTTP/1.0");
    proxy_req_len = strlen(proxy_req);

    // Send the request to the proxy server, return accept if failes
    if (send(proxy_fd, proxy_req, proxy_req_len, 0) != proxy_req_len) {
        syslog(LOG_WARNING, "error while sending request to proxy.");
        close(proxy_fd);
        return 0;
    }

    // Recieve the proxy results
    bzero(proxy_res, sizeof(proxy_res));
    if ((nbytes = recv(proxy_fd, proxy_res, PRXYRES, MSG_WAITALL)) < 1) {
        syslog(LOG_WARNING, "error while receiving response from proxy.");
        close(proxy_fd);
        return 0;
    }

    close(proxy_fd);
 
    // check if the proxy is denying the page by the matched string
    proxy_res[nbytes] = '\0';
    if((strstr(proxy_res, proxy_deny_pattern)) != NULL) {
       return 1;
    } else {
       return 0;
    }

    return 0;
}

