/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * squidguard.h: squidguard backend
 */

extern int squidguard_getfd(FILE *sg_fd[2]);
extern int squidguard_closefd(FILE *sg_fd[2]);
extern int squidguard_backend(FILE *sg_fd[2], char srcip[15], char srcusr[URL_SIZE], char url[URL_SIZE], char *sg_redirect, int debug);
