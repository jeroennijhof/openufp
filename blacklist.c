/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * blacklist.c: blacklist backend
 */

#include "openufp.h"

int blacklist_backend(char *blacklist, char blacklist_url[URL]) {
    char line[256];
    FILE *fd;
    int linenum = 0;

    fd = fopen(blacklist, "r");
    if (fd == NULL) {
        syslog(LOG_WARNING, "blacklist: could not open file %s", blacklist);
        return 0;
    }

    while (fgets(line, 256, fd) != NULL) {
        char url[256];

        linenum++;
        if (line[0] == '#' || line[0] == '\n') continue;

        if (sscanf(line, "%s", url) != 1) {
            syslog(LOG_WARNING, "blacklist: syntax error, line %d\n", linenum);
            continue;
        }
        if((strstr(blacklist_url, url)) != NULL) {
            return 1;
        }
    }
    return 0;
}

