/* openufp server
 *
 * author: Jeroen Nijhof
 * license: GPL v3.0
 *
 * This server translates n2h2 or websense requests to different backends.
 * Frontends supported: n2h2, websense
 * Backends supported: proxy
 * 
 * Proxy: this backend will recieve url get requests from this server and
 *        when the proxy server response contains the PROXY_DENY_PATTERN
 *        a n2h2 or websense deny response will be sent and if not an allow response.
 *
 */

#include "openufp.h"

// Helper functions
void usage() {
    printf("Usage: openufp [OPTIONS] <-n|-w> <BACKEND>\n");
    printf("Example: openufp -n -p '192.168.1.10:3128:Access Denied.'\n");
    printf("Example: openufp -n -f blacklist -p '192.168.1.10:3128:Access Denied.'\n");
    printf("Example: openufp -C http://www.test.com\n\n");
    printf("OPTIONS:\n");
    printf("   -l PORT   on which port openufp will listen for incoming requests\n");
    printf("   -r URL    when url is denied the client will be redirected to this url; n2h2 only\n");
    printf("   -c SECS   cache expire time in seconds; default 3600; 0 disables caching\n");
    printf("   -C URL    remove specified URL from cache\n");
    printf("   -d LEVEL  debug level 1-3\n\n");
    printf("FRONTEND:\n");
    printf("   -n        act as n2h2 server\n");
    printf("   -w        act as websense server\n");
    printf("BACKEND:\n");
    printf("   -p IP:PORT:DENY_PATTERN   use the proxy backend\n");
    printf("             IP is the ipnumber of the proxy server\n");
    printf("             PORT is the portnumber where the proxy server is listening on\n");
    printf("             DENY_PATTERN is a piece of text that should match the deny page\n");
    printf("   -f FILE   use the blacklist file backend\n");
    printf("             FILE is a file which contains blacklisted urls\n");
    printf("   -g        use the squidGuard backend\n\n");
    printf("NOTE:\n");
    printf("   The default location of the cache db is /var/cache/openufp/cache.db.\n");
    printf("   When squidguard backend is used be sure that this program has rw permissions\n");
    printf("   to the squidguard db files.\n\n");
    printf("Version: %s\n", VERSION);
    printf("Report bugs to: jeroen@nijhofnet.nl\n");
}


// Main function
int main(int argc, char**argv) {
    int openufp_fd;
    pid_t pid, child_pid;
    struct sockaddr_in openufp_addr;
    int local_port = 0;
    char *redirect_url = NULL;
    int cache_exp_secs = 3600;
    int debug = 0;
    int frontend = 0;
    char *proxy_ip = NULL;
    int proxy_port = 0;
    char *proxy_deny_pattern = NULL;
    char *blacklist = NULL;
    int squidguard = 0;

    int c;
    while ((c = getopt(argc, argv, "l:r:c:C:d:nwp:f:g")) != -1) {
        char *p;
        DB *cachedb;
        int ret = 0;
        switch(c) {
            case 'l':
                local_port = atoi(optarg);
                break;
            case 'r':
                redirect_url = optarg;
                break;
            case 'c':
                cache_exp_secs = atoi(optarg);
                break;
            case 'C':
                cachedb = open_cache();
                if (rm_cache(cachedb, optarg, 255) == -1)
                    ret = 1;
                close_cache(cachedb, 0);
                exit(ret);
                break;
            case 'd':
                debug = atoi(optarg);
                break;
            case 'n':
                frontend = N2H2;
                break;
            case 'w':
                frontend = WEBSNS;
                break;
            case 'p':
                p = strtok(optarg, ":");
                if (p == NULL)
                    break;
                proxy_ip = p;
                p = strtok (NULL, ":");
                if (p == NULL)
                    break;
                proxy_port = atoi(p);
                p = strtok (NULL, ":");
                if (p == NULL)
                    break;
                proxy_deny_pattern = p;
                break;
            case 'f':
                blacklist = optarg;
                break;
            case 'g':
                squidguard = 1;
                break;
            default:
                usage();
                exit(1);
        }
    }
    if (frontend == 0 || ((proxy_ip == NULL || proxy_port == 0 || proxy_deny_pattern == NULL)
                    && blacklist == NULL && squidguard == 0)) {
        usage();
        exit(1);
    }

    // SIGCHLD handler for stupid childs
    signal(SIGCHLD, SIG_IGN);

    openufp_fd = socket(AF_INET,SOCK_STREAM,0);
    if (openufp_fd < 0) {
        close(openufp_fd);
        printf("openufp v%s: socket failed.\n", VERSION);
        exit(1);
    }

    int optval = 1;
    if (setsockopt(openufp_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        close(openufp_fd);
        printf("openufp v%s: setsockopt failed.\n", VERSION);
        exit(1);
    }

    bzero(&openufp_addr, sizeof(openufp_addr));
    openufp_addr.sin_family = AF_INET;
    openufp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (local_port == 0) {
        if (frontend == N2H2) {
            local_port = 4005;
        } else {
            local_port = 15868;
        }
    }
    openufp_addr.sin_port = htons(local_port);

    if (bind(openufp_fd, (struct sockaddr *)&openufp_addr, sizeof(openufp_addr)) < 0) {
        close(openufp_fd);
        printf("openufp v%s: bind failed.\n", VERSION);
        exit(1);
    }

    if (listen(openufp_fd, 1024) < 0) {
        close(openufp_fd);
        printf("openufp v%s: listen failed.\n", VERSION);
        exit(1);
    }

    printf("openufp v%s: started.\n", VERSION);
    openlog("openufp", LOG_PID|LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "v%s: Jeroen Nijhof <jeroen@nijhofnet.nl>", VERSION); 
    syslog(LOG_INFO, "started listening on %d, waiting for requests...", local_port); 

    if ((pid = fork()) == 0) {
        struct sockaddr_in cli_addr;
        socklen_t cli_size;
        int cli_fd;

        for(;;) {
            cli_size = sizeof(cli_addr);
            cli_fd = accept(openufp_fd, (struct sockaddr *)&cli_addr, &cli_size);
            syslog(LOG_INFO, "client connection accepted.");

            if ((child_pid = fork()) == 0) {
                close(openufp_fd);
                int nbytes = 0;
                int denied = 0;
                char mesg[REQ];
                struct uf_request request;
                FILE *sg_fd[2];

                if (squidguard)
                    squidguard_getfd(sg_fd);

                DB *cachedb = NULL;
                if (cache_exp_secs > 0)
                    cachedb = open_cache();
                else
                    syslog(LOG_INFO, "caching disabled.");

                int cached = 0;
                for(;;) {
                    bzero(&mesg, sizeof(mesg));
                    nbytes = recvfrom(cli_fd, mesg, REQ, 0, (struct sockaddr *)&cli_addr, &cli_size);
                    if (nbytes < 1) {
                        syslog(LOG_WARNING, "connection closed by client.");
                        if (squidguard)
                            squidguard_closefd(sg_fd);
                        close_cache(cachedb, debug);
                        close(cli_fd);
                        exit(1);
                    }

                    if (frontend == N2H2) {
                        request = n2h2_request(mesg);
                    } else {
                        request = websns_request(mesg);
                    }

                    // Alive Request
                    if (request.type == N2H2ALIVE) {
                        if (debug > 2)
                            syslog(LOG_INFO, "received alive request, sending alive response.");
                        n2h2_alive(cli_fd, cli_addr, request.id);
                    }

                    // URL Request
                    if (request.type == N2H2REQ || request.type == WEBSNSREQ) {
                        if (debug > 0)
                            syslog(LOG_INFO, "received url request.");

                        // check if cached
                        cached = in_cache(cachedb, request.url, cache_exp_secs, debug);
                        if (cached == -1) // Happens when there is a cache problem
                            cached = 0;

                        // parse url to blacklist
                        if (!cached && !denied && blacklist != NULL) {
                            denied = blacklist_backend(blacklist, request.url, debug);
                        }

                        // parse url to proxy
                        if (!cached && !denied && proxy_ip != NULL) {
                            denied = proxy_backend(proxy_ip, proxy_port, proxy_deny_pattern, request.url, debug);
                        }

                        // parse url to proxy
                        if (!cached && !denied && squidguard) {
                            denied = squidguard_backend(sg_fd, request.srcip, request.url, debug);
                        }

                        if (denied) {
                            if (frontend == N2H2) {
                                n2h2_deny(cli_fd, cli_addr, request.id, redirect_url);
                            } else {
                                websns_deny(cli_fd, cli_addr, request.id);
                            }
                            if (debug > 0)
                                syslog(LOG_INFO, "url denied: srcip %s, dstip %s, url %s.", request.srcip, request.dstip, request.url);
                        } else {
                            if (frontend == N2H2) {
                                n2h2_accept(cli_fd, cli_addr, request.id);
                            } else {
                                websns_accept(cli_fd, cli_addr, request.id);
                            }
                            if (!cached)
                                add_cache(cachedb, request.url, debug);
                            if (debug > 0)
                                syslog(LOG_INFO, "url accepted: srcip %s, dstip %s, url %s.", request.srcip, request.dstip, request.url);
                        }
                        // reset denied
                        denied = 0;
                    }
                }
                if (squidguard)
                    squidguard_closefd(sg_fd);
                close_cache(cachedb, debug);
            }
            close(cli_fd);
        }
    }
    close(openufp_fd);
    closelog();
    return 0;
}
