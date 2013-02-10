#ifndef __NP_H__
#define __NP_H__

/* Include standard headers */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error( int status, int err, const char *fmt, ... );
void debug(const char *fmt, ...);
uint16_t in_chksum(uint16_t *addr, int len);
void set_address( char *hname, char *sname, struct sockaddr_in *sap, char *protocol );
int udt_send(int sockfd, char *pkt, unsigned int pktLength);
int tselect( int maxp1, fd_set *re, fd_set *we, fd_set *ee);
void start_timer(unsigned int msec );
void stop_timer();
#endif  /* __NP_H__ */
