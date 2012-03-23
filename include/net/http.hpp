#ifndef NET_HTTP_HPP_
#define NET_HTTP_HPP_
#include "common.hpp"

std::string http_get(
	const std::string& uri, const std::string& headers = "",
	const std::string& serv = "80", bool ssl = false);

inline std::string https_get(
	const std::string& uri, const std::string& headers = "")
	{return http_get(uri, headers, "443", true);}

#endif
