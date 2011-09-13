/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * blacklist.c: blacklist backend
 */

#include "openufp.h"

int blacklist_backend(char *blacklist, char url[URL_SIZE], int debug) {
    char line[URL_SIZE];
    FILE *fd = NULL;
    int linenum = 0;

    fd = fopen(blacklist, "r");
    if (fd == NULL) {
        syslog(LOG_WARNING, "blacklist: could not open file %s.", blacklist);
        return 0;
    }

    while (fgets(line, sizeof(line)-1, fd) != NULL) {
        char blacklist_url[URL_SIZE];

        linenum++;
        if (line[0] == '#' || line[0] == '\n') continue;

        if (sscanf(line, "%s", blacklist_url) != 1) {
            syslog(LOG_WARNING, "blacklist: syntax error, line %d.", linenum);
            continue;
        }
        if (debug > 2)
            syslog(LOG_INFO, "blacklist: checking if url contains (%s).", blacklist_url);
        if ((strstr(url, blacklist_url)) != NULL) {
            if (debug > 0)
                syslog(LOG_INFO, "blacklist: url blocked.");
            fclose(fd);
            return 1;
        }
    }
    fclose(fd);
    return 0;
}

