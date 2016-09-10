#ifndef __SOCKET_H__
#define __SOCKET_H__

/*
 * socket.h
 *
 * Routines that call the Spectranet ROM socket library.
 *
 * 2008-05-03 Dylan Smith
 */

/* Definitions */
#define AF_INET		0
#define SOCK_STREAM	1
#define SOCK_DGRAM	2
#define SOCK_RAW	3

/* Much of this should ultimately end up in sys/types.h */
#define in_addr_t	unsigned long

/* Structures */

/* As defined in http://tools.ietf.org/html/draft-ietf-sip-bsd-systems-00 */

struct in_addr
{
	in_addr_t s_addr;		/* 32 bits */
};

struct sockaddr_in	/* internet socket address structure */
{
	int sin_family;			/* offset 0 */
	unsigned int sin_port;		/* offset 2 */
	struct in_addr sin_addr;	/* offset 4 */
	char sin_zero[8];		/* offset 8 */
};


#define sockaddr sockaddr_in
#define socklen_t int

extern int socket(int domain, int type, int protocol);
extern int  sockclose(int fd);
extern int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
extern int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen);
extern int send(int sockfd, void *buf, int len, int flags);
extern int recv(int sockfd, void *buf, int len, int flags);
extern int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int listen(int sockfd, int backlog);


/* htons is a no-op, since all the calls convert machine byte order to
 * network byte order. The macro is provided for compatibility */
#define htons(a)		(a)
#define ntohs(a)		(a)

#endif

