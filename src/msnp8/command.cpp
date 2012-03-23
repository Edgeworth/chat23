#include <pcrecpp.h>
#include <glibmm/regex.h>
#include "msnp8/msnp8.hpp"

namespace MSNP8 {

extern size_t async_NS_size, async_SB_size;
extern std::pair<std::string, sigc::slot<void, Command&, MSN&> > async_NS[];
extern std::pair<std::string, sigc::slot<void, Command&, MSN&, Ptr(Chat) > > async_SB[];

void execAsyncNS(Command& com, MSN& msn) {
	static std::map<std::string, sigc::slot<void, Command&, MSN&> > mapping(async_NS, async_NS+async_NS_size);
	if (mapping.find(com[0]) == mapping.end()) {
		LOG(COMP("Unsupported/invalid command %1", com[0]), LOG_DEBUG)
		return;
	}
	LOG(COMP("Executing asynchronous NS command '%1'", com[0]), LOG_DEBUG)
	mapping[com[0]](com, msn);
}

void execAsyncSB(Command& com, MSN& msn, Ptr(Chat) chat) {
	static std::map<std::string, sigc::slot<void, Command&, MSN&, Ptr(Chat) > > mapping(async_SB, async_SB+async_SB_size);
	if (mapping.find(com[0]) == mapping.end()) {
		LOG(COMP("Unsupported/invalid command %1", com[0]), LOG_DEBUG)
		return;
	}
	LOG(COMP("Executing asynchronous SB command '%1'", com[0]), LOG_DEBUG)
	mapping[com[0]](com, msn, chat);
}

void splitCommands(std::string& combuf, std::queue<Command>& queue) {
	while (true) {
		size_t end = combuf.find("\r\n"), payload;
		if (end == std::string::npos) return;
		Command com = Glib::Regex::split_simple(" ", combuf.substr(0, end));
		end += 2;

		if (combuf.substr(0, 3) == "MSG") {
			com.resize(5);
			payload = strToInt(com[3]);
			com[4] = combuf.substr(end, end+payload);
			if ((end += payload) > combuf.size()) return;
		}

		for (int i = 0; i < com.size(); ++i)
			com[i] = fromURI(com[i]);

		queue.push(com);
		combuf = combuf.substr(end);
	}
}
}
