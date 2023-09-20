#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>


#include "./config.h"

char *printable_addr(struct sockaddr * saddr, socklen_t saddr_len);
char *ret_ipaddr(const char *iface);
int close_connection(int sfd);
int init_socket();
int send_file(const char *pathname, int out_fd);
