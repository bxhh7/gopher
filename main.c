#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <libgen.h>
#include <err.h>

#include "./config.h"
#include "./socket.h"
#include "./types.h"

#define LF 10
#define CR 13
#define CRLF 1310

char *ipaddr     = NULL;
char *port       = NULL;
char *gopher_dir = NULL;
char *net_ifcae  = NULL;
char *hostname   = NULL;


void 
remove_newline(char *buffer) {
	char *ptr;

	ptr = strrchr(buffer, 10);
	if (ptr != NULL)
		*(ptr - 1) = '\0';
}

int 
is_blank(char *buffer) {
	if (buffer[0] == CR && buffer[1] == LF)
		return 1;
	return 0;
}

void
become_daemon() {
	int nullfd;

	setsid();
	switch (fork ()) {
		case 0: setsid();               break;
		default: exit(EXIT_SUCCESS);    break;
	}

	for (int i = 0; i < 100; i++)
		close(i);

	nullfd = open("/dev/null", O_RDWR);
	dup2(nullfd, STDIN_FILENO);
	dup2(nullfd, STDOUT_FILENO);
	dup2(nullfd, STDERR_FILENO);
}

void 
usage_error(const char *program_name) {
	fprintf(stderr, "USAGE: %s [OPTIONS] \n", program_name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "--root-path=path       -r: set the root directory to path (default=/var/gopher).\n");
	fprintf(stderr, "--net-iface=if         -i: set network inteface (the ip addr of this iface will be used as hostname)-\n");
	fprintf(stderr, "                           if hostname was specified, this options is ignored.\n");
	fprintf(stderr, "--port=port            -p: set the port to bind to (default=70).\n");
	fprintf(stderr, "--hostname=hostname    -n: set the hostname. if specified, net-iface is ignored.\n");
	fprintf(stderr, "--generate             -g: generate a gophermap from the root-path and write it to ./gophermap(will overwrite).\n");
	fprintf(stderr, "--verbose              -v: be more verbose.\n");
	fprintf(stderr, "--quiet                -q: be more quiet.\n");
	fprintf(stderr, "--chroot               -c: chroot to root-path.\n");
	fprintf(stderr, "--deaemon              -d: disconnect the ctty and become daemon.\n");
	fprintf(stderr, "--help                 -h: show this text\n");
}


int
send_dir(char *path, int cfd, int gophermap_flag) {
	struct dirent *dt;
	DIR *dp;
	char line[PATH_MAX + 200], _realpath[PATH_MAX];
	char *fail_msg = "ERROR.\n";
	dp = opendir(path);

	if (dp == NULL) {
		write(cfd, fail_msg, strlen(fail_msg));
		return -1;
	}

	chdir(path);
	if (!access(GOPHER_INDEX, R_OK) && gophermap_flag) {
		/* if there is a gophermap under that directory, 
		 * send it. Otherwise generate a gophermap from the files
		 * under that directory */
		send_file(GOPHER_INDEX, cfd);
		return 0;
	}
	while ((dt = readdir(dp)) != NULL) {
		realpath(dt->d_name, _realpath);
		sprintf(line, "%c%s\t%s\t%s\t%s\n", ret_type(dt->d_name), dt->d_name, _realpath, ipaddr, port);
		//printf("sending line=%s", line);
		write(cfd, line, strlen(line));
	}
	closedir(dp);
	return 0;
}


void 
handle_connection(char *buffer, int cfd) {
	if (is_blank(buffer)) {  /* if buffer == CR-LF */
		/* if there isn't any gophermap, generate a gophermap from current directory,
		 * otherwise just show the gophermap using send_file() */
		switch (access(GOPHER_INDEX, R_OK)) {
			case 0:  send_file(GOPHER_INDEX, cfd);   break;
			default: send_dir(gopher_dir , cfd, 1);  break;
		}
	} else {
		remove_newline(buffer);

		/* The client requested for a directory or a file */
		if (is_dir(buffer)) {
			send_dir(buffer, cfd, 1);
		} else {
			send_file(buffer, cfd);
		}
	}
}


	int 
main (int argc, char **argv) 
{
	int sfd, cfd, opt, option_index = 0;
	bool verbose, quiet, chroot_r, gen_gm;
	struct sockaddr_storage claddr;
	char buffer[BUFFER_SIZE];
	socklen_t claddr_len;
	static struct option long_options[] = {
		{"root-path",   required_argument, 0, 'r'},
		{"net-int",     required_argument, 0, 'i'},
		{"port",        required_argument, 0, 'p'},
		{"hostname",    required_argument, 0, 'n'},
		{"generate",    no_argument,       0, 'g'},
		{"verbose",     no_argument,       0, 'v'},
		{"quiet",       no_argument,       0, 'q'},
		{"chroot",      no_argument,       0, 'c'},
		{"daemon",      no_argument,       0, 'd'},
		{"help",        no_argument,       0, 'h'},
		{0,             0,                 0,  0}
	};


	/* Default values */
	gopher_dir      = strdup(GOPHER_DIR);
	port            = strdup(PORT);
	net_ifcae       = strdup(NET_IFACE);
	verbose         = false;
	quiet           = true;
	chroot_r        = false;


	while ((opt = getopt_long(argc, argv, "r:i:p:n:gvqcd", long_options, &option_index)) != -1) {
		switch(opt) {
			case 'r': free(gopher_dir);     gopher_dir = optarg;            break;
			case 'i': free(net_ifcae);      net_ifcae  = optarg;            break;
			case 'p': free(port);           port       = optarg;            break;
			case 'n': hostname = optarg;                                    break;
			case 'g': gen_gm   = true;                                      break;
			case 'v': verbose  = true;      quiet = !verbose;               break;
			case 'q': quiet    = true;      verbose = !quiet;               break;
			case 'c': chroot_r = true;                                      break;
			case 'd': become_daemon();                                      break;
			case 'h': usage_error(argv[0]); exit(EXIT_SUCCESS);             break;
			default:
					  usage_error(argv[0]);
					  exit(EXIT_FAILURE);
					  break;
		}
	}

	if (hostname == NULL) { 
		ipaddr = ret_ipaddr(net_ifcae);
	}
	else { 
		ipaddr = hostname;
	}

	if (gen_gm) {
		int fd, s;
		fd = open(GOPHER_INDEX, 
				O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR |S_IRGRP|S_IWGRP |S_IROTH|S_IWOTH);
		if (fd == -1) 
			err(EXIT_FAILURE, "open");

		s = send_dir(gopher_dir, fd, 0);
		errno = (s == 0) ? 0 : errno;
		printf("%s:%s\n", (s == 0) ? "DONE" : "ERROR", strerror(errno));
		exit(EXIT_SUCCESS);
	}


	sfd = init_socket();
	if (sfd == -1)
		exit(EXIT_FAILURE);

	if (chdir(gopher_dir) == -1) {
		err(EXIT_FAILURE, NULL);
	}

	if (chroot_r) {
		if (chroot(gopher_dir) == -1)
			err(EXIT_FAILURE, "chroot");
		free(gopher_dir);
		gopher_dir = strdup("/");
	}


	signal(SIGCHLD, SIG_IGN);

	for (;;) {
		claddr_len = sizeof(claddr);
		cfd = accept(sfd, (struct sockaddr *)&claddr, &claddr_len);
		if (cfd == -1) {
			if (verbose)
				fprintf(stderr, "Giving up on a client (err=%d)\n", errno);
			continue;
		}


		if (verbose) {
			char *addr = printable_addr((struct sockaddr *)&claddr, claddr_len);
			printf("Connection from %s\n", addr);
			free(addr);
		}

		read(cfd, buffer, BUFFER_SIZE);
		char *arg = strdup(buffer);
		memset(buffer, 0, sizeof(buffer));


		switch (fork()) {
			case 0: 
				handle_connection(arg, cfd);
				free(arg);

				if (verbose)
					puts("Closing connection");

				close_connection(cfd);
				close(cfd);
				exit(EXIT_SUCCESS);
			default: /* close the socket fd in parent and keep on accepting clients */
				close(cfd);
				continue;
		}
	}
}
