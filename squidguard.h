/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * squidguard.h: squidguard backend
 */

extern int squidguard_getfd(FILE *sg_fd[2]);
extern int squidguard_closefd(FILE *sg_fd[2]);
extern int squidguard_backend(FILE *sg_fd[2], char srcip[IP], char url[URL], int debug);

