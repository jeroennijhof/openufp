/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * squidguard.c: squidguard backend
 */

#include "openufp.h"

int squidguard_backend(char srcip[IP], char url[URL], int debug) {
    int outfd[2];
    int infd[2];

    int oldstdin, oldstdout;

    if (pipe(outfd) == -1) {
        syslog(LOG_WARNING, "squidguard: pipe failed.");
        return 0;
    }
    if (pipe(infd) == -1) {
        syslog(LOG_WARNING, "squidguard: pipe failed.");
        return 0;
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
            return 0;
        }
    } else {
        char redirect_url[URL];
        FILE *fp;

        close(0);
        close(1);
        dup2(oldstdin, 0);
        dup2(oldstdout, 1);

        close(outfd[0]);
        close(infd[1]);

        fp = fdopen(outfd[1], "w");
        fprintf(fp, "%s%s%s%s", url, " ", srcip, "/ - - GET");
        fclose(fp);

        bzero(redirect_url, sizeof(redirect_url));
        fp = fdopen(infd[0], "r");
        while (fgets(redirect_url, sizeof(redirect_url)-1, fp) != NULL) {
            if (debug > 1)
                syslog(LOG_INFO, "squidguard: redirect_url (%s).", redirect_url);
            if ((strstr(redirect_url, "http")) != NULL) {
                if (debug > 0)
                    syslog(LOG_INFO, "squidguard: url blocked.");
                fclose(fp);
                return 1;
            }
        }
        fclose(fp);
    }

    return 0;
}

