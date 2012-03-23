#include "gui.hpp"

int main(int argc, char *argv[]) {
	logger.level = LOG_DEBUG;
	Gtk::Main gtk(argc, argv);
	GUI gui;
	gui.run();
}
