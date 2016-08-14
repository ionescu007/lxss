#pragma once

#include <sys/types.h>
#include <pthread.h>

size_t read(int fd, void* buf, size_t count);
size_t write(int fd, const void* buf, size_t count);
int ioctl(int d, int request, ...);
int open(const char* pathname, int flags);
int fcntl(int fd, int cmd, ...);

#define F_SETFD 2

#define O_WRONLY 0x01
#define O_RDWR   0x02
#define O_DIRECT 0x80000


#define	SOCK_STREAM	1		/* stream socket */
#define	AF_UNIX		1		/* local to host (pipes, portals) */

typedef unsigned int socklen_t;
typedef unsigned short sa_family_t;

struct sockaddr_un {
    sa_family_t sun_family;               /* AF_UNIX */
    char        sun_path[108];            /* pathname */
};


struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int
socket(int	domain, int type, int protocol);

int listen(int sockfd, int backlog);

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);;

int close(int fd);

int unlink(const char *pathname);

pid_t getpid(void);

# define RTLD_NEXT	((void *) -1l)

void *dlsym(void *handle, const char *symbol);

int fchmod(int fd, mode_t mode);

pid_t fork(void);

int pause(void);

