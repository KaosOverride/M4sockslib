#include "socket.h"
#include "netdb.h"

#include "M4io.h"

char rxbuffer[2098];
char txbuffer[2098];


int socket(int domain, int type, int protocol)
{
return;
}
int  sockclose(int fd)
{
return;
}
int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
return;
}
int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)
{
return;
}
int send(int sockfd, void *buf, int len, int flags)
{
return;
}
int recv(int sockfd, void *buf, int len, int flags)
{
return;
}
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
return;
}
int listen(int sockfd, int backlog)
{
return;
}




//extern struct hostent *gethostbyname(char *name)
struct hostent *gethostbyname(char *name)
{
M4_IOwrite();

return;
}

