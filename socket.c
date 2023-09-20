#include "./socket.h"
int init_socket() 
{
	struct addrinfo hints, *result, *rp;
	int s, sockfd, opt = 1;

	hints.ai_flags = AI_PASSIVE|AI_NUMERICSERV;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_canonname = NULL;
	hints.ai_protocol = 0;
	hints.ai_addr = NULL;


	s = getaddrinfo(NULL, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;

		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		close(sockfd);
	}
	/*if (rp == NULL) {
	  fprintf(stderr, "Could not bind to any addresses: %s\n", strerror(errno));
	  freeaddrinfo(result);
	  return -1;
	  }*/
	if (listen(sockfd, BACKLOG) == -1)
		return -1;

	freeaddrinfo(result);

	return sockfd;
}

char *
printable_addr(struct sockaddr * saddr, socklen_t saddr_len) {
	char host[NI_MAXHOST], serv[NI_MAXSERV], res[NI_MAXHOST+NI_MAXSERV + 2];
	int s;
	s = getnameinfo(saddr, saddr_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV);
	if (s != 0) {
		sprintf(res, "UNKNKOWN:UNKNOWN");
	} else {
		sprintf(res, "%s:%s", host, serv);
	}

	return strdup(res);
}

int 
close_connection(int sfd) {
	return (shutdown(sfd, SHUT_RDWR));
}

int 
send_file(const char *pathname, int out_fd) {
	ssize_t nread;
	char buffer[512];
	int fd = open(pathname, O_RDONLY);
	if (fd == -1)
		return -1;

	while ((nread = read(fd, buffer, sizeof(buffer))) > 0) {
		write(out_fd, buffer, nread);
	}
	close(fd);
	return 0;
}

char *ret_ipaddr(const char *iface) {

	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	return (inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}
