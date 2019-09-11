/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * squidguard.c: squidguard backend
 */

#include "openufp.h"

int squidguard_backend(char srcip[16], char srcusr[URL_SIZE], char url[URL_SIZE], char *sg_redirect, int debug) {

    FILE *sg_fd;
    char redirect_url[URL_SIZE];

    //Check user; if empty, use ip only:
    if (strlen(srcusr) < 1) {
       if (debug > 2) {
           syslog(LOG_INFO, "squidguard input: username missing, defaulting to IP notation");
       }
       srcusr[0] = '-';
       srcusr[1] = '\0';
    }

    if (debug > 2) {
        syslog(LOG_INFO, "squidguard: url check using ip and user: ip: %s user: %s for url %s", srcip, srcusr, url);
    }

    //Updated fd management to popen():
    char cmd[URL_SIZE];

    snprintf(cmd, URL_SIZE, "echo '%s %s/ - - GET' | /usr/bin/squidGuard", url, srcip);
    sg_fd = popen(cmd, "r");

    if (sg_fd == NULL) {
        syslog(LOG_WARNING, "squidguard: couldn't popen() for output. Verify squidGuard: /usr/bin/squidGuard");
        return 0;
    }

    while (fgets(redirect_url, URL_SIZE, sg_fd) != NULL) {
        if (debug > 2) {
            syslog(LOG_INFO, "squidguard: redirect_url length: %d, post fgets: %s", (int)strlen(redirect_url), redirect_url);
        }

        if (strstr(redirect_url, "url=") != NULL) {
            char *parse;
            parse = strtok(redirect_url, "\"");
            parse = strtok(NULL, "\"");
            strcpy(sg_redirect, parse);

            if (debug > 0)
                syslog(LOG_INFO, "squidguard: url blocked. parsed_redirect: %s, sg_redirectURL: %s", parse, sg_redirect);

            pclose(sg_fd);
            return 1;
        }
        if (debug > 0)
            syslog(LOG_INFO, "squidguard: url accepted.");

        pclose(sg_fd);
        return 0;

    }
    pclose(sg_fd);
    return 0;
}

