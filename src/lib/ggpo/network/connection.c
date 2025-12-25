#include "connection.h"

#include "sys/socket.h"
#include <fcntl.h> // to set nonblocking socket
#include <arpa/inet.h> // htonl

struct _conn_Socket
{
	// empty, socket descriptor is stored directly in the pointer to this struct.
};

struct _conn_Address
{
	struct sockaddr_in sa;
};

conn_Socket conn_open(uint16 port)
{
	// Create socket
#if defined(_WINDOWS)
	SOCKET s = NULL;
#else
	int s = 0;
#endif
	s = socket(AF_INET, SOCK_DGRAM, 0);
	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);
#if defined(_WINDOWS)
	// It seems that on Linux DONTLINGER is the default.
	setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof optval);
#endif

	// Set it to non-blocking
#if defined(_WINDOWS)
	long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode);
#else
	int flags = fcntl(s, F_GETFL, 0);
	ASSERT(flags != -1);
	flags = (flags | O_NONBLOCK);
	fcntl(s, F_SETFL, flags);
#endif

	// Bind it to the specified port
	struct sockaddr_in sin = {0};
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);
	if (bind(s, (struct sockaddr*)&sin, sizeof sin) < 0) {
#if defined(_WINDOWS)
		closesocket(s);
#else
		close(s);
#endif
		return NULL;
	}

	Log("Udp bound to port: %d.\n", port);
	return (conn_Socket)(uptr)s;
}

void conn_close(conn_Socket socket)
{
#if defined(_WINDOWS)
	SOCKET s = socket;
	closesocket(s);
#else
	int s = (int)(uptr)socket;
	close(s);
#endif
}

void conn_send(conn_Socket socket, conn_Address remote, void const *data, uint32 size, int flags)
{
#if defined(_WINDOWS)
	SOCKET s = socket;
#else
	int s = (int)(uptr)socket;
#endif

	// NOTE: sockaddr_in and sockaddr have the same length by design.
	int res = sendto(s, data, size, flags, (struct sockaddr*)&remote->sa, sizeof(remote->sa));
	if (res < 0) {
#if defined(_WINDOWS)
	 	DWORD err = WSAGetLastError();
	 	Log("unknown error in sendto (erro: %d  wsaerr: %d).\n", res, err);
#endif
	 	ASSERT(false && "Unknown error in sendto");
	}
	char dst_ip[1024];
	Log("sent packet length %d to %s:%d (ret:%d).\n",
	    size,
	    inet_ntop(AF_INET, (void*)&remote->sa, dst_ip, ARRAY_SIZE(dst_ip)),
	    ntohs(remote->sa.sin_port),
	    res);
}

int conn_receive(conn_Socket socket, uint8 *buf, uint32 size, conn_Address *out_address)
{
#if defined(_WINDOWS)
	SOCKET s = socket;
#else
	int s = (int)(uptr)socket;
#endif

	// TODO: handle len == 0... indicates a disconnect.
	struct sockaddr_in sender_addr = {0};
	uint32 sender_addr_len = sizeof(sender_addr);
	int len = recvfrom(s, buf, size, 0, (struct sockaddr*)&sender_addr, &sender_addr_len);
	if (len == -1) {
#if defined(_WINDOWS)
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK) {
			Log("recvfrom WSAGetLastError returned %d (%x).\n", error, error);
		}
#endif
	} else if (len > 0) {
		char src_ip[1024];
		Log("recvfrom returned (len:%d  from:%s:%d).\n",
		    len,
		    inet_ntop(AF_INET, (void*)&sender_addr.sin_addr, src_ip, ARRAY_SIZE(src_ip)),
		    ntohs(sender_addr.sin_port));
	}

	return len;
}

bool conn_support_ip_port()
{
	return true;
}

conn_Address conn_address_from_ip_port(char *ip, uint16 port)
{
	struct _conn_Address result = {0};
	result.sa.sin_family = AF_INET; // IPv4
	result.sa.sin_port = port;
	inet_pton(AF_INET, ip, &result.sa.sin_addr);
	return NULL;
}

bool conn_addr_is_equal(conn_Address a, conn_Address b)
{
	return (memcpy(&a->sa.sin_addr, &b->sa.sin_addr, sizeof(a->sa.sin_addr)) == 0)
		&& a->sa.sin_port == b->sa.sin_port;
}
