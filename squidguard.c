/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * squidguard.c: squidguard backend
 */

#include "openufp.h"

int squidguard_getfd(FILE *sg_fd[2]) {
    int outfd[2];
    int infd[2];

    int oldstdin, oldstdout;

    if (pipe(outfd) == -1) {
        syslog(LOG_WARNING, "squidguard: pipe failed.");
        return -1;
    }
    if (pipe(infd) == -1) {
        syslog(LOG_WARNING, "squidguard: pipe failed.");
        return -1;
    }

    oldstdin = dup(0);
    oldstdout = dup(1);

    close(0);
    close(1);

    dup2(outfd[0], 0);
    dup2(infd[1], 1);

    if (!fork()) {
        char *argv[] = { "/usr/bin/squidGuard", 0 };

        close(outfd[0]);
        close(outfd[1]);
        close(infd[0]);
        close(infd[1]);
        close(2);

        if (execv(argv[0], argv) == -1) {
            syslog(LOG_WARNING, "squidguard: failed executing /usr/bin/squidGuard.");
            return -1;
        }
    } else {
        close(0);
        close(1);
        dup2(oldstdin, 0);
        dup2(oldstdout, 1);

        close(outfd[0]);
        close(infd[1]);

        sg_fd[0] = fdopen(infd[0], "r");
        sg_fd[1] = fdopen(outfd[1], "w");
        return 0;
    }
    return 0;
}

int squidguard_closefd(FILE *sg_fd[2]) {
    fclose(sg_fd[0]);
    fclose(sg_fd[1]);
    return 0;
}

int squidguard_backend(FILE *sg_fd[2], char srcip[16], char srcusr[URL_SIZE], char url[URL_SIZE], char *sg_redirect, int debug) {
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

    if (sg_fd[1] == NULL) {
        syslog(LOG_WARNING, "squidguard: could not open fd for input.");
        return 0;
    }

    fprintf(sg_fd[1], "%s %s/ %s - GET\n", url, srcip, srcusr);
    fflush(sg_fd[1]);

    if (sg_fd[0] == NULL) {
        syslog(LOG_WARNING, "squidguard: could not open fd for output.");
        return 0;
    }
    while (fgets(redirect_url, URL_SIZE, sg_fd[0]) != NULL) {
        if (strlen(redirect_url) > 2) {
            char *parse;
            parse = strtok (redirect_url, " ");
            strcpy(sg_redirect, parse);

            if (debug > 0)
                syslog(LOG_INFO, "squidguard: url blocked. parsed_red: %s -- sg_redirectURL: %s", parse, sg_redirect );

            return 1;
        }
        if (debug > 0)
            syslog(LOG_INFO, "squidguard: url accepted.");
        return 0;
    }
    return 0;
}

