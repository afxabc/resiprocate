#include "buffer.h"
#include "netbase.h"
#include "inetaddress.h"

int fd_nonblock(FD fd, int enable)
{
#ifdef WIN32
    u_long param = enable;
    return ioctlsocket(fd, FIONBIO, &param);
#else
	int param = fcntl(fd, F_GETFL);
	if (enable)
		param |= O_NONBLOCK;
	else param &= ~O_NONBLOCK;
	return fcntl(fd, F_SETFL, param);
#endif
}

void sock_init()
{
#ifdef WIN32

	static bool doneInit = false;

	if (doneInit)
		return;
	doneInit = true;

	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData;
	int err = WSAStartup(wVersionRequested, &wsaData );
	if (err != 0)
	{
//		assert(0); // is this is failing, try a different version that 2.2, 1.0 or later will likely work
		exit(1);
	}

#endif
}

int sock_resuseaddr(FD fd, int enable)
{
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
}

int sock_bind(FD fd, const InetAddress& addr)
{
#ifdef WIN32
	BOOL val = TRUE;
	if (addr.getIPInt() == 0)
		setsockopt(fd, SOL_SOCKET, ~SO_REUSEADDR, (char*)&val, sizeof(val));
	else sock_resuseaddr(fd, 1);
#else
	sock_resuseaddr(fd, 1);
#endif
	return bind( fd, addr.getSockaddr(), sizeof(sockaddr_in));
}

int sock_sendto(FD fd, const InetAddress& peer, const char* data, int len)
{
	int slen = sendto(fd, data, len, 0, peer.getSockaddr(), sizeof(sockaddr_in));
	
#ifdef WIN32
	int err = WSAGetLastError();
	if (slen < 0 && err != WSAEWOULDBLOCK)
		slen = -1;
	else if (slen < 0)
		slen = 0;
#else
	int err = errno;
	if (slen < 0)
	{
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            slen = -1;
        else slen = 0;
	}
#endif

	return slen;
}

int sock_recvfrom(FD fd, InetAddress& from, Buffer& buffer)
{
	int addrlen = sizeof(sockaddr_in);
	int ret = recvfrom(fd, buffer.beginWrite(), buffer.writableBytes(), 0, from.getSockaddr(), (socklen_t*)&addrlen);
	if (ret > 0)
		buffer.writerIndexMove(ret);

	return ret;
}

int sock_recvfrom(FD fd, InetAddress& from, char* buf, int size)
{
	int addrlen = sizeof(sockaddr_in);
	return recvfrom(fd, buf, size, 0, from.getSockaddr(), (socklen_t*)&addrlen);
}

int sock_accept(FD fd, InetAddress& peer)
{
	int addrlen = sizeof(sockaddr_in);
	int ret = accept(fd, peer.getSockaddr(), (socklen_t*)&addrlen);
	if (ret <= 0)
		ret = INVALID_FD;
	return ret;
}

int sock_connect(FD fd, InetAddress& peer)		//0:OK; 1:try; -1:error
{
#ifdef WIN32
	int ret = connect(fd, peer.getSockaddr(), sizeof(sockaddr_in));
//	assert(ret <= 0);

	int err = WSAGetLastError();
	if (ret == 0 || err == WSAEISCONN)
		ret = 0;
	else if (err == WSAEWOULDBLOCK || err == WSAEALREADY)
		ret = 1;
	else
		ret = -1;
#else
	int ret = connect(fd, peer.getSockaddr(), sizeof(sockaddr_in));
	assert(ret <= 0);

	int err = errno;
	if (ret == 0 || err == EISCONN)
		ret = 0;
	else if (err == EINPROGRESS || err == EALREADY)
		ret = 1;
	else
		ret = -1;
#endif


	return ret;
}

int sock_send(FD fd, const Buffer& buff)		//>0:OK; =0:try; <0:error
{
	int slen = send(fd, buff.beginRead(), buff.readableBytes(), 0);

#ifdef WIN32
	int err = WSAGetLastError();
	if (slen < 0 && err != WSAEWOULDBLOCK)
		slen = -1;
	else if (slen < 0)
		slen = 0;
#else
	int err = errno;
	if (slen < 0)
	{
        if (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)
            slen = -1;
        else slen = 0;
	}
#endif

	return slen;
}

unsigned int sock_recvlength(FD fd)
{

#ifdef WIN32
	u_long len = 0;
	ioctlsocket(fd, FIONREAD, &len);
#else
	unsigned int len = 0;
	ioctl(fd, FIONREAD, &len);
#endif

	return len;
}

#ifdef WIN32

int sock_poll(struct pollfd *fds, nfds_t numfds, int timeout)
{
	fd_set read_set;
	fd_set write_set;
	fd_set exception_set;
	nfds_t i;
	int n;
	int rc;

	if (numfds >= FD_SETSIZE)
	{
//		LOGE("poll error : numfds %d >= %d !", numfds, FD_SETSIZE);
		return -1;
	}

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&exception_set);

	n = 0;
	for (i = 0; i < numfds; i++)
	{
		if (fds[i].fd == INVALID_FD)
			continue;

		if (fds[i].events & POLLIN)
			FD_SET(fds[i].fd, &read_set);
		if (fds[i].events & POLLOUT)
			FD_SET(fds[i].fd, &write_set);
		if (fds[i].events & POLLERR)
			FD_SET(fds[i].fd, &exception_set);

		if (fds[i].fd >= n)
			n = fds[i].fd + 1;
	}

	if (n == 0)
		/* Hey!? Nothing to poll, in fact!!! */
		return 0;

	if (timeout >= 0)
	{
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = 1000 * (timeout % 1000);
		rc = select(n, &read_set, &write_set, &exception_set, &tv);
	}
	else rc = select(n, &read_set, &write_set, &exception_set, NULL);

	if (rc < 0)
		return rc;

	for (i = 0; i < numfds; i++)
	{
		fds[i].revents = 0;

		if (FD_ISSET(fds[i].fd, &read_set))
			fds[i].revents |= POLLIN;
		if (FD_ISSET(fds[i].fd, &write_set))
			fds[i].revents |= POLLOUT;
		if (FD_ISSET(fds[i].fd, &exception_set))
			fds[i].revents |= POLLERR;
	}

	return rc;
}
#else

int sock_poll(struct pollfd *fds, nfds_t numfds, int timeout)
{
	return poll(fds, numfds, timeout);
}

#endif

