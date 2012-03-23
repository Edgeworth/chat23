#ifndef GUI_HPP_
#define GUI_HPP_

#include <pcrecpp.h>
#include <gtkmm.h>
#include "msnp8/msnp8.hpp"

const int POOL_MAX = 5;

struct ChatPage {
	Ptr(MSNP8::Chat) chat;
	Glib::RefPtr<Gtk::Builder> builder;
	Gtk::HBox *hboxTab;
	Gtk::Label *labelTab;
	Gtk::Button *buttonTab;
	Gtk::VPaned *vpaneChat;
	Gtk::TextView *textChat;
	Gtk::TextView *textChatEntry;
	std::string single;
};

class GUI {
public:
	GUI();

	void run();
private:
	bool progressBarPulse() {progressBarConnecting->pulse(); return true;}
	void buttonLoginClicked();
	void sendMessage(Ptr(ChatPage) page);
	void scrollBottom(Gtk::TextView* view);
	void populateChats();
	void removePage(Ptr(ChatPage) p) {
		noteMain->remove_page(*p->vpaneChat);
		msn.closeChat(p->chat);
	}
	void replenishPool();
	void openChat();
	void textChatEntrySend(Ptr(ChatPage) page, const Gtk::TextBuffer::iterator&, const Glib::ustring& text, int bytes);

	void on_msn_error(const std::string& err);
	void on_finish_login();
	void on_contact_update(Ptr(MSNP8::Contact) cont);
	void on_group_update(int grp);
	void on_invite_chat(Ptr(MSNP8::Chat) chat);
	void on_create_chat(Ptr(MSNP8::Chat) chat);
	void on_join_chat(Ptr(MSNP8::Chat) chat, std::string joined);
	void on_left_chat(Ptr(MSNP8::Chat) chat, std::string left);
	void on_chat_msg(Ptr(MSNP8::Chat) chat, std::string sender, std::string msg);
	void on_chat_close(Ptr(MSNP8::Chat) chat);

	Glib::RefPtr<Gtk::Builder> builder;
	Gtk::Window contWindow, *chatWindow;
	Gtk::MessageDialog *dialogError;

	Gtk::VBox *vboxLogin;
	Gtk::Button *buttonLogin;
	Gtk::Entry *entryLoginEmail, *entryLoginPassword;

	Gtk::VBox *vboxConnecting;
	Gtk::ProgressBar *progressBarConnecting;

	Gtk::VBox *vboxContact;
	Gtk::TreeView *treeViewContact;
	Gtk::Button *buttonContact;
	Gtk::Notebook *noteMain;

	MSNP8::MSN msn;
	Glib::Thread *msn_thread;
	std::queue<std::vector<std::string> > chat_queue;
	std::map<Ptr(MSNP8::Chat), Ptr(ChatPage)> chats;
	std::vector<Ptr(MSNP8::Chat)> chat_pool;
	int chat_requests;

	struct ModelColumns : public Gtk::TreeModelColumnRecord {
	public:
		ModelColumns()
			{add(status); add(name); add(email); add(visible);}

		Gtk::TreeModelColumn<std::string> status;
		Gtk::TreeModelColumn<std::string> name;
		Gtk::TreeModelColumn<std::string> email;
		Gtk::TreeModelColumn<bool> visible;
	} cols;
	Glib::RefPtr<Gtk::ListStore> liststore;
	Glib::RefPtr<Gtk::TreeModelFilter> filter;
};

#endif
