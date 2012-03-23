#include <map>
#include <utility>
#include "msnp8/error.hpp"

static std::pair<std::string, std::string> errors[] = {
	std::make_pair("200", "Invalid Syntax"),
	std::make_pair("201", "Invalid parameter"),
	std::make_pair("205", "Invalid principal"),
	std::make_pair("206", "Domain name missing"),
	std::make_pair("207", "Already logged in"),
	std::make_pair("208", "Invalid principal"),
	std::make_pair("209", "Nickname change illegal"),
	std::make_pair("210", "Principal list full"),
	std::make_pair("213", "Invalid rename request?"),
	std::make_pair("215", "Principal already on list"),
	std::make_pair("216", "Principal not on list"),
	std::make_pair("217", "Principal not online"),
	std::make_pair("218", "Already in mode"),
	std::make_pair("219", "Principal is in the opposite list"),
	std::make_pair("223", "Too many groups"),
	std::make_pair("224", "Invalid group"),
	std::make_pair("225", "Principal not in group"),
	std::make_pair("227", "Group not empty"),
	std::make_pair("228", "Group with same name already exists"),
	std::make_pair("229", "Group name too long"),
	std::make_pair("230", "Cannot remove group zero"),
	std::make_pair("231", "Invalid group"),
	std::make_pair("240", "Empty domain"),
	std::make_pair("280", "Switchboard failed"),
	std::make_pair("281", "Transfer to switchboard failed"),
	std::make_pair("282", "P2P Error"),
	std::make_pair("300", "Required field missing"),
	std::make_pair("302", "Not logged in"),
	std::make_pair("402", "Error accessing contact list"),
	std::make_pair("403", "Error accessing contact list"),
	std::make_pair("420", "Invalid Account Permissions"),
	std::make_pair("500", "Internal server error"),
	std::make_pair("501", "Database server error"),
	std::make_pair("502", "Command disabled"),
	std::make_pair("510", "File operation failed"),
	std::make_pair("511", "Banned"),
	std::make_pair("520", "Memory allocation failed"),
	std::make_pair("540", "Challenge response failed"),
	std::make_pair("600", "Server is busy"),
	std::make_pair("601", "Server is unavailable"),
	std::make_pair("602", "Peer nameserver is down"),
	std::make_pair("603", "Database connection failed"),
	std::make_pair("604", "Server is going down"),
	std::make_pair("605", "Server unavailable"),
	std::make_pair("700", "Could not create connection"),
	std::make_pair("710", "Bad CVR parameters sent"),
	std::make_pair("711", "Write is blocking"),
	std::make_pair("712", "Session is overloaded"),
	std::make_pair("713", "Calling too rapidly"),
	std::make_pair("714", "Too many sessions"),
	std::make_pair("715", "Not expected"),
	std::make_pair("717", "Bad friend file"),
	std::make_pair("731", "Not expected"),
	std::make_pair("800", "Changing too rapidly"),
	std::make_pair("910", "Server too busy"),
	std::make_pair("911", "Server is busy"),
	std::make_pair("912", "Server too busy"),
	std::make_pair("913", "Not allowed when hidden"),
	std::make_pair("914", "Server unavailable"),
	std::make_pair("915", "Server unavailable"),
	std::make_pair("916", "Server unavailable"),
	std::make_pair("917", "Authentication failed"),
	std::make_pair("918", "Server too busy"),
	std::make_pair("919", "Server too busy"),
	std::make_pair("920", "Not accepting new principals"),
	std::make_pair("921", "Server too busy"),
	std::make_pair("922", "Server too busy"),
	std::make_pair("923", "Kids' Passport without parental consent"),
	std::make_pair("924", "Passport account not yet verified"),
	std::make_pair("928", "Bad ticket"),
	std::make_pair("931", "Account not on this server")
};


namespace MSNP8 {

std::string error(const std::string& code) {
	static std::map<std::string, std::string> mapping(errors, errors+sizeof(errors)/sizeof(*errors));
	if (mapping.find(code) == mapping.end()) return "";
	return mapping[code];
}

}
