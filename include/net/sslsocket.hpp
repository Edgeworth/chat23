#ifndef NET_SSLSOCKET_HPP_
#define NET_SSLSOCKET_HPP_
#include <openssl/ssl.h>
#include "common.hpp"
#include "net/socket.hpp"

class SSLSocket : public Socket {
public:
	SSLSocket() : m_ctx(NULL), m_ssl(NULL) {}
	virtual ~SSLSocket();

	virtual void send(const std::string& msg);
	virtual std::string recv(size_t bytes = 0);
	virtual std::string recvall();

protected:
	virtual void connect(const std::string& h, const std::string& s);

private:
	SSL_CTX *m_ctx;
	SSL *m_ssl;
};

#endif
