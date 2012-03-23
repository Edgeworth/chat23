#ifndef MSNP8_CHAT_HPP_
#define MSNP8_CHAT_HPP_
#include <vector>
#include "common.hpp"
#include "net/socket.hpp"
#include "msnp8/contact.hpp"

namespace MSNP8 {

class Chat {
public:
	void sendMessage(const std::string& msg);
	void invite(const std::vector<std::string>& contacts);

	std::vector<std::string> cont;
private:

	Ptr(Socket) m_sb;
	std::string m_buf;

	friend class ComNS;
	friend class ComSB;
	friend class MSN;
};

}

#endif
