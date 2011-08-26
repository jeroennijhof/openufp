/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * cache.c: cache module
 */

#include "openufp.h"
#define DATABASE "/var/cache/openufp/cache.db"

DB *open_cache() {
    DB *dbp;
    int ret;

    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        syslog(LOG_WARNING, "cache: %s.", db_strerror(ret));
        return NULL;
    }
    if ((ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        syslog(LOG_WARNING, "cache: %s.", db_strerror(ret));
        close_cache(dbp);
        return NULL;
    } 
    return dbp;
}

int close_cache(DB *dbp) {
    if (dbp == NULL) {
        syslog(LOG_WARNING, "cache: critical db problem, caching disabled.");
        return -1;
    }
    return dbp->close(dbp, 0);
}

int in_cache(DB *dbp, char url[URL], int expire_sec, int debug) {
    DBT key, data;
    int ret;
    char sec[15];
    sprintf(sec, "%ld", time(NULL) - expire_sec);

    if (dbp == NULL) {
        syslog(LOG_WARNING, "cache: critical db problem, caching disabled.");
        return 0;
    }

    bzero(&key, sizeof(key));
    bzero(&data, sizeof(data));
    key.data = url;
    key.size = strlen(url)+1;
    data.data = sec;
    data.size = strlen(sec)+1;

    if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
        if (strcmp((char *)data.data, sec) > 0) {
            if (debug > 0)
                syslog(LOG_INFO, "cache: url %s retrieved, time %s expire at %s.",
                        (char *)key.data, (char *)data.data, sec);
            return 1;
        }
        if (debug > 0)
            syslog(LOG_INFO, "cache: url in cache expired.");
    }
    if (debug > 0)
        syslog(LOG_INFO, "cache: url not in cache.");
    return 0;
}

int add_cache(DB *dbp, char url[URL], int debug) {
    DBT key, data;
    int ret;
    char sec[15];
    sprintf(sec, "%ld", time(NULL));

    if (dbp == NULL) {
        syslog(LOG_WARNING, "cache: critical db problem, caching disabled.");
        return 0;
    }

    bzero(&key, sizeof(key));
    bzero(&data, sizeof(data));
    key.data = url;
    key.size = strlen(url)+1;
    data.data = sec;
    data.size = strlen(sec)+1;

    if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {
        dbp->sync(dbp, 0);
        if (debug > 0)
            syslog(LOG_INFO, "cache: key %s stored.", (char *)key.data);
        return 0;
    }
    syslog(LOG_WARNING, "cache: %s.", db_strerror(ret));
    return -1;
}

int rm_cache(DB *dbp, char url[URL], int debug) {
    DBT key;
    int ret;

    if (dbp == NULL) {
        if (debug == 255)
            printf("cache: critical db problem, caching disabled.\n");
        syslog(LOG_WARNING, "cache: critical db problem, caching disabled.");
        return 0;
    }

    bzero(&key, sizeof(key));
    key.data = url;
    key.size = strlen(url)+1;

    if ((ret = dbp->del(dbp, NULL, &key, 0)) == 0) {
        dbp->sync(dbp, 0);
        if (debug > 0)
            syslog(LOG_INFO, "cache: key %s removed.", (char *)key.data);
        if (debug == 255)
            printf("cache: key %s removed.\n", (char *)key.data);
        return 0;
    }
    syslog(LOG_WARNING, "cache: %s.", db_strerror(ret));
    if (debug == 255)
        printf("cache: %s.\n", db_strerror(ret));
    return -1;
}

