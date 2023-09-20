#ifndef __CONFIG__H
#define __CONFIG__H

#define PORT 		"70"                    /* Default port */
#define BACKLOG         10                      
#define GOPHER_INDEX	"gophermap"              
#define GOPHER_DIR	"/var/gopher"	        /* Default root-path*/
#define BUFFER_SIZE 	1024
#define NET_IFACE       "lo"                    /* Default net-iface */


extern char *ipaddr; 
extern char *port;    
extern char *gopher_dir; 
extern char *net_ifcae;
extern char *hostname; 


#endif
/* See types.c */
