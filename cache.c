/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * cache.c: cache module
 */

#include "openufp.h"
#define DATABASE "/var/cache/openufp/cache.db"

void get_hash(const char *s, char hash[10]) {
    unsigned int _hash = 0;
    int c;

    while((c = *s++)) {
        /* hash = hash * 33 ^ c */
        _hash = ((_hash << 5) + _hash) ^ c;
    }
    snprintf(hash, sizeof(hash), "%u", _hash);
}

DB *open_cache() {
    DB *dbp;
    int ret;

    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        syslog(LOG_WARNING, "cache db_create: %s.", db_strerror(ret));
        return NULL;
    }
    if ((ret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        syslog(LOG_WARNING, "cache open: %s.", db_strerror(ret));
        close_cache(dbp, 0);
        return NULL;
    } 
    return dbp;
}

int close_cache(DB *dbp, int debug) {
    if (dbp == NULL) {
        if (debug > 1)
            syslog(LOG_INFO, "cache: close_cache: caching disabled.");
        return -1;
    }
    return dbp->close(dbp, 0);
}

int in_cache(DB *dbp, char hash[10], int expire_sec, int debug) {
    DBT key, data;
    int ret;
    char sec[15];
    sprintf(sec, "%ld", time(NULL) - expire_sec);

    if (dbp == NULL) {
        if (debug > 1)
            syslog(LOG_INFO, "cache: in_cache: caching disabled.");
        return -1;
    }

    bzero(&key, sizeof(key));
    bzero(&data, sizeof(data));
    key.data = hash;
    key.size = strlen(hash)+1;
    data.data = sec;
    data.size = strlen(sec)+1;

    if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
        if (strcmp((char *)data.data, sec) > 0) {
            if (debug > 0)
                syslog(LOG_INFO, "cache: hash %s retrieved, time %s expire at %s.",
                        (char *)key.data, (char *)data.data, sec);
            return 1;
        }
        if (debug > 0)
            syslog(LOG_INFO, "cache: hash in cache expired.");
        rm_cache(dbp, hash, debug);
    }
    if (debug > 0)
        syslog(LOG_INFO, "cache: hash not in cache.");
    return 0;
}

int add_cache(DB *dbp, char hash[10], int debug) {
    DBT key, data;
    int ret;
    char sec[15];
    sprintf(sec, "%ld", time(NULL));

    if (dbp == NULL) {
        if (debug > 1)
            syslog(LOG_INFO, "cache: add_cache: caching disabled.");
        return -1;
    }

    bzero(&key, sizeof(key));
    bzero(&data, sizeof(data));
    key.data = hash;
    key.size = strlen(hash)+1;
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

int rm_cache(DB *dbp, char hash[10], int debug) {
    DBT key;
    int ret;

    if (dbp == NULL) {
        if (debug == 255)
            printf("cache: critical db problem, caching disabled.\n");
        if (debug > 1)
            syslog(LOG_INFO, "cache: rm_cache: caching disabled.");
        return -1;
    }

    bzero(&key, sizeof(key));
    key.data = hash;
    key.size = strlen(hash)+1;

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

