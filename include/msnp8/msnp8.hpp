#ifndef MSNP8_MSNP8_HPP_
#define MSNP8_MSNP8_HPP_
#include <queue>
#include <vector>
#include <map>
#include <algorithm>
#include <glibmm/dispatcher.h>
#include <glibmm/thread.h>
#include "common.hpp"
#include "msnp8/command.hpp"
#include "msnp8/chat.hpp"
#include "msnp8/contact.hpp"
#include "net/http.hpp"

namespace MSNP8 {

class MSN {
public:
	void run(const std::string& email, const std::string& password);
	void quit() {m_running = false;}

	sigc::signal<void> on_login_begin;
	sigc::signal<void> on_login_end;
	sigc::signal<void, const std::string&> on_error;
	sigc::signal<void, Ptr(Contact) > on_cont;
	sigc::signal<void, int> on_grp;
	sigc::signal<void, Ptr(Chat) > on_chat_invite;
	sigc::signal<void, Ptr(Chat) > on_chat_create;
	sigc::signal<void, Ptr(Chat), std::string> on_chat_enter;
	sigc::signal<void, Ptr(Chat), std::string> on_chat_leave;
	sigc::signal<void, Ptr(Chat), std::string, std::string> on_chat_msg;
	sigc::signal<void, Ptr(Chat) > on_chat_close;

	Ptr(Contact) self() {return m_self;}
	Ptr(Contact) contByEmail(const std::string& email);
	std::vector<Ptr(Contact) > cont;
	const std::string& groupName(int grp) {return m_grp[grp];}
	void closeChat(Ptr(Chat) chat);
	void startChat() {m_ns->send("XFR 1 SB\r\n");}

private:
	void dispatched(const sigc::slot<void>& sig) {
		sig();
		m_dispatch_connection.disconnect();
		m_mutex->unlock();
	}
	void dispatch(const sigc::slot<void>& sig) {
		m_mutex->lock();
		m_dispatch_connection = m_dispatcher.connect(
			sigc::bind(sigc::mem_fun(*this, &MSN::dispatched), sig));
		m_dispatcher();
	}
	void login(const std::string& password);

	Ptr(Contact) m_self;
	std::map<int, std::string> m_grp;
	std::vector<Ptr(Chat) > m_chat;

	bool m_running;
	Ptr(Socket) m_ns;
	Glib::Dispatcher m_dispatcher;
	Ptr(Glib::Mutex) m_mutex;
	sigc::connection m_dispatch_connection;
	std::queue<Command> m_commands;

	friend class ComNS;
	friend class ComSB;
};

}

#endif
