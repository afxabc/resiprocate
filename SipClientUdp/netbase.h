
#ifndef NET_NET_H__
#define NET_NET_H__

#ifdef WIN32	/*windows*/

#include <winsock2.h>

typedef int socklen_t;

#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR S_IWRITE
#endif

typedef SOCKET FD;
typedef int nfds_t;

#define INVALID_FD INVALID_SOCKET

#define SIG_FD 0xFFFFFFF0


struct pollfdx {
    FD fd;
    short events;  /* events to look for */
    short revents; /* events that occurred */
	WSAEVENT wevent;
};

#if(_WIN32_WINNT < 0x0600)
/* events & revents */
#define POLLIN     0x0001  /* any readable data available */
#define POLLOUT    0x0002  /* file descriptor is writeable */
#define POLLRDNORM POLLIN
#define POLLWRNORM POLLOUT
#define POLLRDBAND 0x0008  /* priority readable data */
#define POLLWRBAND 0x0010  /* priority data can be written */
#define POLLPRI    0x0020  /* high priority readable data */

/* revents only */
#define POLLERR    0x0004  /* errors pending */
#define POLLHUP    0x0080  /* disconnected */
#define POLLNVAL   0x1000  /* invalid file descriptor */
#endif

#define POLLCONN   0x0040  /* for win32: wait for connect */
#define POLLWRITE  0x0400  /* for win32: trigger to write */

inline int closefd(FD f)
{ 
	return closesocket(f);
}

#else	/*linux*/

#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>


typedef int FD;
#define INVALID_FD -1

inline int closefd(FD f)
{ 
	return close(f);
}

struct pollfdx {
    FD fd;
    short events;  /* events to look for */
    short revents; /* events that occurred */
};

#endif

class InetAddress;
class Buffer;

void sock_init();

int fd_nonblock(FD fd, int enable);
int sock_resuseaddr(FD fd, int enable);
int sock_bind(FD fd, const InetAddress& addr);

int sock_sendto(FD fd, const InetAddress& peer, const char* data, int len);
int sock_recvfrom(FD fd, InetAddress& from, Buffer& buf);
int sock_recvfrom(FD fd, InetAddress& from, char* buf, int size);

int sock_accept(FD fd, InetAddress& peer);
int sock_connect(FD fd, InetAddress& peer);
int sock_send(FD fd, const Buffer& buff);

int sock_poll(struct pollfd *fds, nfds_t numfds, int timeout);

unsigned int sock_recvlength(FD fd);

#endif
