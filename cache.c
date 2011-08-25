/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * cache.c: cache module
 */

#include "openufp.h"
#define DATABASE "cache.db"

DB *open_cache() {
    DB *dbp;
    int ret;

    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        syslog(LOG_WARNING, "cache: open_cache: %s.", db_strerror(ret));
        return NULL;
    }
    if ((ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        syslog(LOG_WARNING, "cache: open_cache: %s.", db_strerror(ret));
        close_cache(dbp);
        return NULL;
    } 
    return dbp;
}

int close_cache(DB *dbp) {
    return dbp->close(dbp, 0);
}

int in_cache(DB *dbp, char url[URL], int expire_sec) {
    DBT key, data;
    int ret;
    char sec[15];
    sprintf(sec, "%ld", time(NULL) - expire_sec);

    bzero(&key, sizeof(key));
    bzero(&data, sizeof(data));
    key.data = url;
    key.size = strlen(url)+1;
    data.data = sec;
    data.size = strlen(sec)+1;

    if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
        syslog(LOG_INFO, "cache: in_cache: %s: key retrieved: time was %s: expire at %s\n",
                    (char *)key.data, (char *)data.data, sec);
        if (strcmp((char *)data.data, sec) > 0) {
            return 1;
        }
    }
    syslog(LOG_WARNING, "cache: in_cache: %s.", db_strerror(ret));
    return 0;
}

int add_cache(DB *dbp, char url[URL]) {
    DBT key, data;
    int ret;
    char sec[15];
    sprintf(sec, "%ld", time(NULL));

    bzero(&key, sizeof(key));
    bzero(&data, sizeof(data));
    key.data = url;
    key.size = strlen(url)+1;
    data.data = sec;
    data.size = strlen(sec)+1;

    if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {
        dbp->sync(dbp, 0);
        syslog(LOG_INFO, "cache: add_cache: %s key stored.\n", (char *)key.data);
        return 0;
    }
    syslog(LOG_WARNING, "cache: add_cache: %s.", db_strerror(ret));
    return 0;
}

int flush_cache(DB *dbp) {
    return 0;
}

