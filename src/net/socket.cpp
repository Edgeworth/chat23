#include <cstring>
#include <algorithm>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include "net/socket.hpp"
#include "net/sslsocket.hpp"

const int RECV_BUF = 1024;

Ptr(Socket) tcp(const std::string& hs, bool ssl, int tries) {
	size_t split = hs.find(':');
	ERR(split == std::string::npos,
		COMP("Malformed ip address:%1", hs))
	return tcp_s(hs.substr(0, split), hs.substr(split+1));
}

Ptr(Socket) tcp_s(const std::string& h, const std::string& s, bool ssl, int tries) {
	for (int i = 0; i < tries; ++i) {
		LOG(COMP("Trying connection (%1/%2)", i+1, tries), LOG_MSG)
		try {
			Ptr(Socket) socket(ssl ? new SSLSocket() : new Socket());
			socket->connect(h, s);
			return socket;
		} catch (...) {}
	}
	ERR(true, COMP("Could not connect after %1 tries.", tries))
	return Ptr(Socket)((Socket*)NULL);
}

void Socket::connect(const std::string& h, const std::string& s) {
	host = h; serv = s;

	int status;
	addrinfo hint, *info;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;

	LOG(COMP("Connecting to %1", hostserv()), LOG_MSG)

	ERR(status = getaddrinfo(host.c_str(), serv.c_str(), &hint, &info),
		COMP("getaddrinfo():%1", gai_strerror(status)))

	ERR((fd = socket(info->ai_family, info->ai_socktype,
		info->ai_protocol)) == -1, COMP("socket():%1", strerror(errno)))

	ERR(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(status=1),
		sizeof(status)) == -1, COMP("setsockopt():%1", strerror(errno)))

	ERR(::connect(fd, info->ai_addr, info->ai_addrlen) == -1,
		COMP("connect():%1", strerror(errno)))

	freeaddrinfo(info);
	disconnected = false;
}

void Socket::send(const std::string& msg) {
	int sent = 0;
	for (int i = 0; i < msg.size(); i += sent) {
		ERR((sent = ::send(fd, &msg[i], msg.size()-i, 0)) == -1,
			COMP("send():%1", strerror(errno)))
	}
	LOG(COMP("Sent %1B to %2:\n'%3'",
		msg.size(), hostserv(), Glib::strescape(msg)), LOG_DEBUG)
}

std::string Socket::recv(size_t bytes) {
	int recvd = 0;

	std::string msg(bytes, '\0');
	for (int i = 0; i < bytes; i += recvd) {
		ERR((recvd = ::recv(fd, &msg[i], bytes-i, 0)) == -1,
			COMP("recv():%1", strerror(errno)))

		if (recvd == 0) {
			disconnected = true;
			return "";
		}
	}

	LOG(COMP("Recvd %1B from %2:\n'%3'",
		msg.size(), hostserv(), Glib::strescape(msg)), LOG_DEBUG)
	return msg;
}

std::string Socket::recvall() {
	int recvd = RECV_BUF;
	char buf[RECV_BUF];
	std::string msg;

	while (recvd == RECV_BUF) {
		ERR((recvd = ::recv(fd, buf, RECV_BUF, 0)) == -1,
			COMP("recv():%1", strerror(errno)))
		msg.append(buf, recvd);
		if (recvd == 0) {
			disconnected = true;
			return "";
		}
	}

	LOG(COMP("Recvd %1B from %2:\n'%3'",
		msg.size(), hostserv(), Glib::strescape(msg)), LOG_DEBUG)
	return msg;
}

