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
extern int in_cache(DB *dbp, char url[URL], int expire_sec);
extern int add_cache(DB *dbp, char url[URL]);
extern int flush_cache(DB *dbp);

