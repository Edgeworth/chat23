#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
#include "net/sslsocket.hpp"

namespace {
const int DESC_BUF = 1024;
const int RECV_BUF = 1024;

int SSLErrorsCB(const char *str, size_t len, void *u) {
	((std::string*)u)->append(str, len);
}

std::string SSLErrors() {
	std::string errs;
	ERR_print_errors_cb(&SSLErrorsCB, &errs);
	return errs;
}

class _SSLInitDeinit {
public:
	_SSLInitDeinit() {
		SSL_library_init();
		SSL_load_error_strings();

		ERR(RAND_poll() == 0, SSLErrors())
		LOG("Seeded the PRNG successfully", LOG_DEBUG)
	}

	~_SSLInitDeinit() {
		ENGINE_cleanup();
		CONF_modules_unload(1);
		CRYPTO_cleanup_all_ex_data();
		RAND_cleanup();
		ERR_free_strings();
		EVP_cleanup();
	}
} _ssl_init_deinit_object;

SSL_CTX *initCTX() {
	SSL_CTX *m_ctx;
	ERR((m_ctx = SSL_CTX_new(SSLv23_method())) == NULL,
		COMP("SSL_CTX_new():%1", SSLErrors()))

	SSL_CTX_set_options(m_ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2);
	ERR(SSL_CTX_set_default_verify_paths(m_ctx) == 0,
		COMP("SSL_CTX_set_default_verify_paths():%1", SSLErrors()))
	return m_ctx;
}
}

SSLSocket::~SSLSocket() {
	if (m_ssl) {
		LOG(COMP("Closing SSL connection to %1", hostserv()), LOG_MSG)
		ERR(SSL_shutdown(m_ssl) == -1, COMP("SSL_shutdown:%1", SSLErrors()))
		SSL_free(m_ssl);
		SSL_CTX_free(m_ctx);
	}
}

void SSLSocket::connect(const std::string& h, const std::string& p) {
	m_ssl = SSL_new(m_ctx = initCTX());
	Socket::connect(h, p);
	LOG(COMP("Using SSL (%1) for connection to %2", SSL_get_version(m_ssl), hostserv()), LOG_MSG)

	ERR(SSL_set_fd(m_ssl, fd) == 0,
		COMP("SSL_set_fd:%1", SSLErrors()))

	ERR(SSL_connect(m_ssl) <= 0,
		COMP("SSL_connect:%1", SSLErrors()))

	LOG(SSL_state_string_long(m_ssl), LOG_MSG)

	char desc_buf[DESC_BUF];
	LOG(COMP("Using cypher:\n%1 bit %2",
		SSL_CIPHER_get_bits(SSL_get_current_cipher(m_ssl), NULL),
		SSL_CIPHER_description(SSL_get_current_cipher(m_ssl), desc_buf, DESC_BUF)), LOG_DEBUG)
}


void SSLSocket::send(const std::string& msg) {
	int sent = 0;
	for (int i = 0; i < msg.size(); i += sent) {
		ERR((sent = SSL_write(m_ssl, &msg[i], msg.size()-i)) <= 0,
			COMP("SSL_write():%1", SSLErrors()))
	}
	LOG(COMP("Sent %1B via SSL (%2) to %3:\n'%4'",
		msg.size(), SSL_get_version(m_ssl), hostserv(), Glib::strescape(msg)), LOG_DEBUG)
}

std::string SSLSocket::recv(size_t bytes) {
	int recvd = 0;

	std::string msg(bytes, '\0');
	for (int i = 0; i < bytes; i += recvd) {
		ERR((recvd = SSL_read(m_ssl, &msg[i], bytes-i)) <= 0,
			COMP("SSL_read():%1", SSLErrors()))
		if (recvd == 0) {
			disconnected = true;
			return "";
		}
	}

	LOG(COMP("Recvd %1B via SSL (%2) from %3:\n'%4'",
		msg.size(), SSL_get_version(m_ssl), hostserv(), Glib::strescape(msg)), LOG_DEBUG)
	return msg;
}

std::string SSLSocket::recvall() {
	int recvd = RECV_BUF;
	char buf[RECV_BUF];
	std::string msg;

	while (recvd == RECV_BUF) {
		ERR((recvd = SSL_read(m_ssl, buf, RECV_BUF)) <= -1,
			COMP("SSL_read():%1", SSLErrors()))
		msg.append(buf, recvd);
		if (recvd == 0) {
			disconnected = true;
			return "";
		}
	}

	LOG(COMP("Recvd %1B via SSL (%2) from %3:\n'%4'",
		msg.size(), SSL_get_version(m_ssl), hostserv(), Glib::strescape(msg)), LOG_DEBUG)
	return msg;
}
