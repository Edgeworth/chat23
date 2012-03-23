#ifndef logger_HPP_
#define logger_HPP_
#include "common.hpp"

enum LogLevel {
	LOG_DEBUG,
	LOG_MSG,
	LOG_WARN,
	LOG_ERR
};

extern class Log {
public:
	Log() : level(LOG_MSG) {}
	void log(const std::string& msg, LogLevel l = LOG_DEBUG);
	void operator()(const std::string& msg, LogLevel l = LOG_DEBUG) {log(msg, l);}

	LogLevel level;
} logger;

#endif
