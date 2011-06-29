/* openufp server
 *
 * author: Jeroen Nijhof
 * version: 1.04
 * license: GPL v3.0
 *
 * proxy.h: proxy backend
 */

#define PRXYRES 1024

extern int proxy_backend(char *proxy_ip, int proxy_port, char *proxy_deny_pattern, char proxy_url[URL]);

