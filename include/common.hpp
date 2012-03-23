#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <string>
#include <cstdlib>
#include <stdexcept>
#include <stdint.h>
#include <stddef.h>
#include <glibmm/ustring.h>
#include <glibmm/uriutils.h>
#include <glibmm/stringutils.h>
#include <boost/current_function.hpp>
#include <boost/shared_ptr.hpp>
#include "log.hpp"

#define COMP Glib::ustring::compose
#define Ptr(x) boost::shared_ptr<x >

inline std::string toURI(const std::string& noturi) {return Glib::uri_escape_string(noturi);}
inline std::string fromURI(const std::string& uri) {return Glib::uri_unescape_string(uri);}
inline int strToInt(const std::string& str) {return atoi(str.c_str());}

#define LOG(msg, level) \
	{std::string func__ = std::string(BOOST_CURRENT_FUNCTION); \
	func__ = func__.substr(0, func__.find('(')); \
	func__ = func__.substr(func__.rfind(' ')+1); \
	std::string msg__ = Glib::ustring::compose("%1():%2", func__, msg); \
	logger(msg__, level);}

#define ERR(cond, msg) \
	{if ((cond)) { \
		std::string func__ = std::string(BOOST_CURRENT_FUNCTION); \
		func__ = func__.substr(0, func__.find('(')); \
		func__ = func__.substr(func__.rfind(' ')+1); \
		std::string msg__ = Glib::ustring::compose("%1():%2", func__, msg); \
		logger(msg__, LOG_ERR); \
		throw std::runtime_error(msg__); \
	}}

#endif
