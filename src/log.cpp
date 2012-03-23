#include <cstdio>
#include "log.hpp"

Log logger;

void Log::log(const std::string& msg, LogLevel l) {
	std::string mapping[] = {"DEBUG", "MSG", "WARN", "ERR"};
	if (l >= level) fprintf(stderr, "%s:%s\n", mapping[l].c_str(), msg.c_str());
	fflush(stderr);
}
