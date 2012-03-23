#include <pcrecpp.h>
#include <glibmm/thread.h>
#include "msnp8/msnp8.hpp"
#include "msnp8/error.hpp"

namespace MSNP8 {
using sigc::signal;
using sigc::mem_fun;
using sigc::bind;

void MSN::run(const std::string& email, const std::string& password) {
	try {
	m_mutex = Ptr(Glib::Mutex)(new Glib::Mutex);
	m_running = true;
	m_self = Ptr(Contact)(new Contact);
	m_self->m_email = email;

	login(password);
	std::string combuf;
	while (m_running) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_ns->fd, &fds);
		int max = m_ns->fd;
		std::map<int, Ptr(Chat) > fdchat;
		for (int i = 0; i < m_chat.size(); ++i) {
			FD_SET(m_chat[i]->m_sb->fd, &fds);
			fdchat[m_chat[i]->m_sb->fd] = m_chat[i];
			if (m_chat[i]->m_sb->fd > max)
				max = m_chat[i]->m_sb->fd;
		}
		select(max+1, &fds, NULL, NULL, NULL);

		for (int i = 0; i < max+1; ++i) {
			if (!FD_ISSET(i, &fds)) continue;
			if (m_ns->fd == i) {
				combuf += m_ns->recvall();
				splitCommands(combuf, m_commands);
				if (m_ns->disconnected) ERR(true, "Disconnected by notification server.")
			} else {
				fdchat[i]->m_buf += fdchat[i]->m_sb->recvall();
				splitCommands(fdchat[i]->m_buf, m_commands);
				if (fdchat[i]->m_sb->disconnected) {
					dispatch(bind(on_chat_close.make_slot(), fdchat[i]));
					m_chat.erase(std::find(m_chat.begin(), m_chat.end(), fdchat[i]));
				}
			}
			while (!m_commands.empty()) {
				if (error(m_commands.front()[0]) != "") LOG(error(m_commands.front()[0]), LOG_ERR)
				if (m_ns->fd == i) execAsyncNS(m_commands.front(), *this);
				else execAsyncSB(m_commands.front(), *this, fdchat[i]);
				m_commands.pop();
			}
		}
	}
	} catch (std::exception& e) {
		dispatch(bind(on_error.make_slot(), e.what()));
	}
}

Ptr(Contact) MSN::contByEmail(const std::string& email) {
	for (int i = 0; i < cont.size(); ++i)
		if (cont[i]->email() == email) return cont[i];
	if (m_self->email() == email) return m_self;
	LOG(COMP("Could not find contact %1", email), LOG_WARN)
	Ptr(Contact) def(new Contact);
	def->m_email = email;
	return def;
}

void MSN::closeChat(Ptr(Chat) chat) {
	std::vector<Ptr(Chat) >::iterator i =
		std::find(m_chat.begin(), m_chat.end(), chat);
	if (i == m_chat.end())
		LOG("Could not close chat (invalid chat)", LOG_ERR)

	m_chat.erase(i);
}

void MSN::login(const std::string& password) {
	dispatch(on_login_begin.make_slot());
	using pcrecpp::RE;

	m_ns = tcp_s("messenger.hotmail.com", "1863");

	xfr:
		m_ns->send("VER 1 MSNP8\r\n");
		ERR(m_ns->recvall() != "VER 1 MSNP8\r\n", "MSNP8 not supported")
		m_ns->send(COMP("CVR 1 0x0409 0 0 0 0 0 0 %1\r\n", m_self->email()));
		m_ns->recvall();
		m_ns->send(COMP("USR 1 TWN I %1\r\n", m_self->email()));

		std::string command = m_ns->recvall();
		if (command.find("XFR") != std::string::npos) {
			std::string redir;
			RE("NS (.*?) 0").PartialMatch(command, &redir);
			m_ns = tcp(redir);
			goto xfr;
		}

	std::string partial_auth, login_server;

	RE("TWN S (.*)\r\n").PartialMatch(command, &partial_auth);
	RE("DALogin=(.*?),").PartialMatch(
			https_get("nexus.passport.com/rdr/pprdr.asp"), &login_server);

	std::string login_reply, ticket;
	redir:
		login_reply = https_get(login_server,
			COMP("Authorization: Passport1.4 OrgVerb=GET,OrgURL="
			"http%%3A%%2F%%2Fmessenger%%2Emsn%%2Ecom,sign-in=%1,pwd=%2,%3",
			toURI(m_self->email()), toURI(password), partial_auth));

		if (login_reply.find("200 OK") != std::string::npos)
			RE("from-PP='(.*?)'").PartialMatch(login_reply, &ticket);

		if (login_reply.find("302 Found") != std::string::npos) {
			RE("Location: (.*?)\r\n").PartialMatch(login_reply, &login_server);
			goto redir;
		}

		ERR(login_reply.find("401 Unauthorized") != std::string::npos,
				"Incorrect password")

	m_ns->send(COMP("USR 1 TWN S %1\r\n", ticket));
	pcrecpp::RE("USR 1 OK .*? (.*?) .*? .*?\r\n").PartialMatch(m_ns->recvall(), &self()->m_name);
	m_ns->send("SYN 1 0\r\n");
	m_ns->send("CHG 1 NLN 0\r\n");

	dispatch(on_login_end.make_slot());
}
}
