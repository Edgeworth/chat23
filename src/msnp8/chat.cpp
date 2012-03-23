#include "msnp8/chat.hpp"

namespace MSNP8 {

void Chat::sendMessage(const std::string& msg) {
	std::string complete = std::string("MIME-Version: 1.0\r\n"
		"Content-Type: text/plain; charset=UTF-8\r\nX-MMS-IM-Format:"
		" FN=Arial; EF=I; CO=0; CS=0;\r\n\r\n") + msg;
	m_sb->send(COMP("MSG 1 N %1\r\n%2", complete.size(), complete));
}

void Chat::invite(const std::vector<std::string>& contacts) {
	for (int i = 0; i < contacts.size(); ++i)
		m_sb->send(COMP("CAL 1 %1\r\n", contacts[i]));
}

}
