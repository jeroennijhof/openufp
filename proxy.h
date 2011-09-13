/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * proxy.h: proxy backend
 */

#define PRXYRES 1024

extern int proxy_backend(char *proxy_ip, int proxy_port, char *proxy_deny_pattern, char url[URL_SIZE], int debug);

