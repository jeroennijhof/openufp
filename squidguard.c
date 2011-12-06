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

int squidguard_backend(FILE *sg_fd[2], char srcip[15], char url[URL_SIZE], int debug) {
    char redirect_url[URL_SIZE];
    //bzero(redirect_url, sizeof(redirect_url));

    fprintf(sg_fd[1], "%s %s/ - - GET\n", url, srcip);
    fflush(sg_fd[1]);
    while (fgets(redirect_url, sizeof(redirect_url)-1, sg_fd[0]) != NULL) {
        if (debug > 1)
            syslog(LOG_INFO, "squidguard: redirect_url (%s).", redirect_url);
        if ((strstr(redirect_url, "http")) != NULL) {
            if (debug > 0)
                syslog(LOG_INFO, "squidguard: url blocked.");
            return 1;
        }
        return 0;
    }
    return 0;
}

