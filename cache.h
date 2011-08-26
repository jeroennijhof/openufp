/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * cache.h: cache module
 */

#include <db.h>

extern DB *open_cache();
extern int close_cache(DB *dbp);
extern int in_cache(DB *dbp, char url[URL], int expire_sec, int debug);
extern int add_cache(DB *dbp, char url[URL], int debug);
extern int rm_cache(DB *dbp, char url[URL], int debug);

