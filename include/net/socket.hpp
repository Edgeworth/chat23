#ifndef NET_SOCKET_HPP_
#define NET_SOCKET_HPP_
#include "common.hpp"

class Socket;

class Socket {
public:
	Socket() : fd(-1), disconnected(true) {}
	virtual ~Socket() {if (fd != -1) close(fd);}

	virtual void send(const std::string& msg);
	virtual std::string recv(size_t bytes = 0);
	virtual std::string recvall();

	std::string hostserv() {return host+":"+serv;}

	int fd;
	bool disconnected;
	std::string host, serv;

protected:
	virtual void connect(const std::string& h, const std::string& s);

	friend Ptr(Socket) tcp_s(const std::string&, const std::string&, bool, int);
};

Ptr(Socket) tcp(const std::string& hs, bool ssl = false, int tries = 3);

Ptr(Socket) tcp_s(const std::string& h, const std::string& s, bool ssl = false, int tries = 3);
#endif
