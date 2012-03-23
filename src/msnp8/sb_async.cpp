#include <pcrecpp.h>
#include "msnp8/msnp8.hpp"

namespace MSNP8 {
class ComSB {
public:

	static void sb_IRO(Command& com, MSN& msn, Ptr(Chat) chat) {
		com.erase(com.begin()+1, com.begin()+4);
		sb_JOI(com, msn, chat);
	}
	static void sb_JOI(Command& com, MSN& msn, Ptr(Chat) chat) {
		chat->cont.push_back(com[1]);
		msn.dispatch(sigc::bind(msn.on_chat_enter.make_slot(), chat, chat->cont.back()));
	}
	static void sb_MSG(Command& com, MSN& msn, Ptr(Chat) chat) {
		if (!pcrecpp::RE("^MIME-Version: 1.0\r\nContent-Type: text/plain;").PartialMatch(com[4])) return;
		std::string msg;
		pcrecpp::RE("\r\n(.*?)$").PartialMatch(com[4], &msg);
		msn.dispatch(sigc::bind(msn.on_chat_msg.make_slot(), chat, com[1], msg));
	}
	static void sb_BYE(Command& com, MSN& msn, Ptr(Chat) chat) {
		for (int i = 0; i < chat->cont.size(); ++i)
			if (chat->cont[i] == com[1]) chat->cont.erase(chat->cont.begin()+i);
		msn.dispatch(sigc::bind(msn.on_chat_leave.make_slot(), chat, com[1]));
	}
};

std::pair<std::string, sigc::slot<void, Command&, MSN&, Ptr(Chat) > > async_SB[] = {
	std::make_pair("IRO", ComSB::sb_IRO),
	std::make_pair("JOI", ComSB::sb_JOI),
	std::make_pair("MSG", ComSB::sb_MSG),
	std::make_pair("BYE", ComSB::sb_BYE),
};

size_t async_SB_size = sizeof(async_SB)/sizeof(*async_SB);

}
