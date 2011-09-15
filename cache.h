/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * cache.h: cache module
 */

#include <db.h>

extern void get_hash(const char *s, char hash[10]);
extern DB *open_cache();
extern int close_cache(DB *dbp, int debug);
extern int in_cache(DB *dbp, char hash[10], int expire_sec, int debug);
extern int add_cache(DB *dbp, char hash[10], int debug);
extern int rm_cache(DB *dbp, char hash[10], int debug);

