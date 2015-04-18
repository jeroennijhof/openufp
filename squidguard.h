/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * squidguard.h: squidguard backend
 */

extern int squidguard_closefd(FILE *sg_fd);
extern int squidguard_backend(char srcip[16], char srcusr[URL_SIZE], char url[URL_SIZE], char *sg_redirect, int debug);
