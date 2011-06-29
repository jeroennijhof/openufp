/* openufp server
 *
 * author: Jeroen Nijhof
 * version: 1.04
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

// Functions
void usage() {
    printf("Usage: openufp [OPTIONS] <-n|-w> <BACKEND>\n");
    printf("Example: openufp -n -p '192.168.1.10:3128:Access Denied.'\n\n");
    printf("OPTIONS:\n");
    printf("   -l PORT   on which port openufp will listen for incoming requests\n");
    printf("   -r URL    when url is denied the client will be redirected to this url; n2h2 only\n");
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
    printf("             FILE is a file which contains blacklisted urls\n\n");
    printf("Version: %s\n", VERSION);
    printf("Report bugs to: jeroen@nijhofnet.nl\n");
}


// Main routine
int main(int argc, char**argv) {
    int openufp_fd, cli_fd;
    struct sockaddr_in openufp_addr, cli_addr;
    socklen_t cli_size;
    int c, nbytes;
    int local_port = 0;
    int debug = 1;
    char *redirect_url = NULL;
    char mesg[REQ];
    struct uf_request request;
    pid_t pid, child_pid;
    int frontend = 0;
    char *p;
    char *proxy_ip = NULL;
    int proxy_port = 0;
    char *proxy_deny_pattern = NULL;
    char *blacklist = NULL;

    while ((c = getopt(argc, argv, "l:r:d:nwp:f:")) != -1) {
        switch(c) {
            case 'l':
                local_port = atoi(optarg);
                break;
            case 'r':
                redirect_url = optarg;
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
            default:
                usage();
                exit(1);
        }
    }
    if (frontend == 0 || ((proxy_ip == NULL || proxy_port == 0 || proxy_deny_pattern == NULL) && blacklist == NULL)) {
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
        for(;;) {
            cli_size = sizeof(cli_addr);
            cli_fd = accept(openufp_fd, (struct sockaddr *)&cli_addr, &cli_size);
            if (debug > 1)
                syslog(LOG_INFO, "client connection accepted");

            if ((child_pid = fork()) == 0) {
                close(openufp_fd);

                for(;;) {
                    bzero(&mesg, sizeof(mesg));
                    nbytes = recvfrom(cli_fd, mesg, REQ, 0, (struct sockaddr *)&cli_addr, &cli_size);
                    if (nbytes < 6) {
                        syslog(LOG_WARNING, "wrong request, closing connection");
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
                            syslog(LOG_INFO, "received alive request, sending alive response");
                        n2h2_alive(cli_fd, cli_addr, request.id);
                    }

                    // URL Request
                    if (request.type == N2H2REQ || request.type == WEBSNSREQ) {
                        if (debug > 1)
                            syslog(LOG_INFO, "received url request");

                        // parse url to proxy
                        if (proxy_ip != NULL) {
                            if (proxy_backend(proxy_ip, proxy_port, proxy_deny_pattern, request.url) == 0) {
                                if (frontend == N2H2) {
                                    n2h2_accept(cli_fd, cli_addr, request.id);
                                } else {
                                    websns_accept(cli_fd, cli_addr, request.id);
                                }
                                if (debug > 1)
                                    syslog(LOG_INFO, "url accepted: srcip %s, dstip %s, url %s", request.srcip, request.dstip, request.url);
                            } else {
                                if (frontend == N2H2) {
                                    n2h2_deny(cli_fd, cli_addr, request.id, redirect_url);
                                } else {
                                    websns_deny(cli_fd, cli_addr, request.id);
                                }
                                if (debug > 1)
                                    syslog(LOG_INFO, "url denied: srcip %s, dstip %s, url %s", request.srcip, request.dstip, request.url);
                            }
                        }

                        // parse url to blacklist
                        if (blacklist != NULL) {
                        }
                    }
                }
            }
            close(cli_fd);
        }
    }
    close(openufp_fd);
    closelog();
    return 0;
}
