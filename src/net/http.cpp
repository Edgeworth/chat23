#include <glibmm/uriutils.h>
#include "net/http.hpp"
#include "net/socket.hpp"

std::string http_get(
	const std::string& uri, const std::string& headers,
	const std::string& serv, bool ssl) {
	std::string host = uri.substr(0, uri.find('/'));
	std::string file = uri.substr(uri.find('/'));

	Ptr(Socket) socket = tcp_s(host, serv, ssl);

	if (ssl) LOG(COMP("Sending HTTPS GET request to %1", socket->hostserv()), LOG_MSG)
	else LOG(COMP("Sending HTTP GET request to %1", socket->hostserv()), LOG_MSG)
	socket->send(COMP("GET %1 HTTP/1.0\r\n%2\r\n\r\n", file, headers));

	return socket->recvall();
}
