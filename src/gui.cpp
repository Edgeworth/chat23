#include "gui.hpp"

static void appendText(Gtk::TextView* view, const std::string& text) {
	view->get_buffer()->set_text(view->get_buffer()->get_text()+text);
}

static std::string getEntry(Gtk::TextView* t) {
	std::string text = t->get_buffer()->get_text();
	t->get_buffer()->set_text("");
	return text;
}

static std::string getContList(Ptr(MSNP8::Chat) chat, MSNP8::MSN& msn) {
	std::string list;
	for (int i = 0; i < chat->cont.size(); ++i) {
		std::string display = msn.contByEmail(chat->cont[i])->name();
		list += (display == "" ? chat->cont[i] : display)+',';
	}
	return list;
}


GUI::GUI() : chat_requests(0), cols(), liststore(Gtk::ListStore::create(cols)),
	filter(Gtk::TreeModelFilter::create(liststore)) {
	Glib::thread_init();
	builder = Gtk::Builder::create_from_file("layout.glade");
	builder->get_widget("vboxLogin", vboxLogin);
	builder->get_widget("buttonLogin", buttonLogin);
	builder->get_widget("entryLoginEmail", 	entryLoginEmail);
	builder->get_widget("entryLoginPassword", entryLoginPassword);

	builder->get_widget("vboxConnecting", vboxConnecting);
	builder->get_widget("progressBarConnecting", progressBarConnecting);

	builder->get_widget("noteMain", noteMain);
	builder->get_widget("vboxContact", vboxContact);
	builder->get_widget("buttonContact", buttonContact);
	builder->get_widget("treeViewContact", treeViewContact);

	builder->get_widget("dialogError", dialogError);
	builder->get_widget("chatWindow", chatWindow);

	buttonLogin->signal_clicked().connect(sigc::mem_fun(*this, &GUI::buttonLoginClicked));
	buttonContact->signal_pressed().connect(sigc::mem_fun(*this, &GUI::openChat));
	Glib::signal_timeout().connect(sigc::bind_return(
		sigc::mem_fun(*progressBarConnecting, &Gtk::ProgressBar::pulse), true), 20);
	contWindow.add(*vboxLogin);

	treeViewContact->set_model(filter);
	treeViewContact->append_column("Status", cols.status);
	treeViewContact->append_column("Name", cols.name);
	treeViewContact->append_column("Email", cols.email);
	treeViewContact->set_search_column(cols.name);
	std::vector<Gtk::TreeViewColumn*> c = treeViewContact->get_columns();
	for (int i = 0; i < c.size(); ++i)
		c[i]->set_resizable(true);
	treeViewContact->get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	filter->set_visible_column(cols.visible);

	msn.on_error.connect(sigc::mem_fun(*this, &GUI::on_msn_error));
	msn.on_login_end.connect(sigc::mem_fun(*this, &GUI::on_finish_login));
	msn.on_cont.connect(sigc::mem_fun(*this, &GUI::on_contact_update));
	msn.on_grp.connect(sigc::mem_fun(*this, &GUI::on_group_update));
	msn.on_chat_invite.connect(sigc::mem_fun(*this, &GUI::on_invite_chat));
	msn.on_chat_create.connect(sigc::mem_fun(*this, &GUI::on_create_chat));
	msn.on_chat_enter.connect(sigc::mem_fun(*this, &GUI::on_join_chat));
	msn.on_chat_leave.connect(sigc::mem_fun(*this, &GUI::on_left_chat));
	msn.on_chat_msg.connect(sigc::mem_fun(*this, &GUI::on_chat_msg));
	msn.on_chat_close.connect(sigc::mem_fun(*this, &GUI::on_chat_close));
}

void GUI::run() {
	Gtk::Main::run(contWindow);
}

void GUI::buttonLoginClicked() {
	msn_thread = Glib::Thread::create(sigc::bind(sigc::mem_fun(msn, &MSNP8::MSN::run),
		entryLoginEmail->get_text(), entryLoginPassword->get_text()), false);
	contWindow.remove();
	contWindow.add(*vboxConnecting);
}

void GUI::sendMessage(Ptr(ChatPage) page) {
	std::string msg = getEntry(page->textChatEntry);
	if (msg[msg.size()-1] == '\n') msg = msg.substr(0, msg.size()-1);

	if (page->chat->cont.size() == 0 && page->single == "") {
		appendText(page->textChat, "You're talking to yourself.\n");
		return;
	}

	appendText(page->textChat, COMP("%1 (%2):%3\n",
		msn.self()->name(), msn.self()->email(), msg));

	if (page->chat->cont.size() == 0 && page->single != "") {
		std::vector<std::string> invites;
		invites.push_back(page->single);
		page->chat->invite(invites);
		page->single = "";
		while (page->chat->cont.size() == 0) Gtk::Main::iteration(true);
	}
	page->chat->sendMessage(msg);
}

void GUI::scrollBottom(Gtk::TextView* view) {
	Gtk::TextIter should_be_unnecessary = view->get_buffer()->end();
	view->scroll_to(view->get_buffer()->create_mark(should_be_unnecessary));
}

void GUI::populateChats() {
	while (!chat_queue.empty() && !chat_pool.empty()) {
		chat_pool.front()->invite(chat_queue.front());
		chat_queue.pop();
		on_invite_chat(chat_pool.front());
		chat_pool.erase(chat_pool.begin());
		--chat_requests;
	}
	replenishPool();
}

void GUI::replenishPool() {
	for (; chat_requests < POOL_MAX; ++chat_requests)
		msn.startChat();
}

void GUI::openChat() {
	std::vector<std::string> invites;
	std::vector<Gtk::TreeModel::Path> paths = treeViewContact->get_selection()->get_selected_rows();
	for (int i = 0; i < paths.size(); ++i)
		invites.push_back((*filter->get_iter(paths[i]))[cols.email]);
	chat_queue.push(invites);
	populateChats();
}

void GUI::textChatEntrySend(Ptr(ChatPage) page, const Gtk::TextBuffer::iterator&, const Glib::ustring& text, int bytes) {
	if (text.size() == 1 && text[0] == '\n') sendMessage(page);
}
void GUI::on_msn_error(const std::string& err) {
	contWindow.remove();
	contWindow.add(*vboxLogin);
	dialogError->set_secondary_text(err);
	dialogError->run();
	dialogError->hide();
}
void GUI::on_finish_login() {
	contWindow.remove();
	contWindow.add(*vboxContact);
	replenishPool();
}
void GUI::on_contact_update(Ptr(MSNP8::Contact) cont) {
	std::string map[] = {"Online", "Busy", "Idle", "BRB", "Away", "Phone", "Lunch", "Offline"};
	for (Gtk::TreeModel::Children::iterator i = liststore->
		children().begin();i != liststore->children().end(); ++i) {
		if (cont->email() == std::string((*i)[cols.email])) {
			(*i)[cols.status] = map[cont->status()];
			(*i)[cols.name] = cont->name();
			(*i)[cols.visible] = (cont->status() != MSNP8::OFFLINE);
			filter->refilter();
			return;
		}
	}
	Gtk::TreeModel::iterator row = liststore->append();
	(*row)[cols.status] = map[cont->status()];
	(*row)[cols.name] = cont->name();
	(*row)[cols.email] = cont->email();
	(*row)[cols.visible] = (cont->status() != MSNP8::OFFLINE);
	filter->refilter();
}
void GUI::on_group_update(int grp) {
}

void GUI::on_invite_chat(Ptr(MSNP8::Chat) chat) {
	Ptr(ChatPage) page(new ChatPage);
	page->builder = Gtk::Builder::create_from_file("msn.glade");
	page->builder->get_widget("hboxTab", page->hboxTab);
	page->builder->get_widget("labelTab", page->labelTab);
	page->builder->get_widget("buttonTab", page->buttonTab);
	page->builder->get_widget("vpaneChat", page->vpaneChat);
	page->builder->get_widget("textChat", page->textChat);
	page->builder->get_widget("textChatEntry", page->textChatEntry);
	page->chat = chat;
	chats[chat] = page;


	page->labelTab->set_text("...");
	noteMain->append_page(*page->vpaneChat, *page->hboxTab);
	noteMain->set_menu_label_text(*page->vpaneChat, "...");
	page->textChat->get_buffer()->signal_changed().connect(
		sigc::bind(sigc::mem_fun(*this, &GUI::scrollBottom), page->textChat));
	page->textChatEntry->get_buffer()->signal_insert().connect(sigc::bind<0>(
		sigc::mem_fun(*this, &GUI::textChatEntrySend), page));
	page->buttonTab->signal_clicked().connect(sigc::bind(
		sigc::mem_fun(*this, &GUI::removePage), page));
	chatWindow->show();
}

void GUI::on_create_chat(Ptr(MSNP8::Chat) chat) {
	chat_pool.push_back(chat);
	populateChats();
}

void GUI::on_join_chat(Ptr(MSNP8::Chat) chat, std::string joined) {
	appendText(chats[chat]->textChat,
		COMP("%1 (%2) has joined the chat.\n",
		msn.contByEmail(joined)->name(), joined));
	chats[chat]->labelTab->set_text(getContList(chat, msn));
	noteMain->set_menu_label_text(*chats[chat]->vpaneChat, getContList(chat, msn));
}
void GUI::on_left_chat(Ptr(MSNP8::Chat) chat, std::string left) {
	appendText(chats[chat]->textChat,
		COMP("%1 (%2) has left the chat.\n",
		msn.contByEmail(left)->name(), left));
	if (chat->cont.size() == 0) {
		chats[chat]->single = left;
		return;
	}
	chats[chat]->labelTab->set_text(getContList(chat, msn));
	noteMain->set_menu_label_text(*chats[chat]->vpaneChat, getContList(chat, msn));
}
void GUI::on_chat_msg(Ptr(MSNP8::Chat) chat, std::string sender, std::string msg) {
	appendText(chats[chat]->textChat, COMP("%1 (%2):%3\n",
		msn.contByEmail(sender)->name(), sender, msg));
}

void GUI::on_chat_close(Ptr(MSNP8::Chat) chat) {
	std::vector<Ptr(MSNP8::Chat) >::iterator i =
		std::find(chat_pool.begin(), chat_pool.end(), chat);
	if (i != chat_pool.end()) {
		chat_pool.erase(i);
		--chat_requests;
		replenishPool();
	}
}
