#ifndef MSNP8_COMMAND_UTIL_HPP_
#define MSNP8_COMMAND_UTIL_HPP_
#include <queue>
#include "common.hpp"

namespace MSNP8 {
class MSN;
class Chat;
typedef std::vector<std::string> Command;

void execAsyncNS(Command& com, MSN& msn);
void execAsyncSB(Command& com, MSN& msn, Ptr(Chat) chat);
void splitCommands(std::string& combuf, std::queue<Command>& queue);
}

#endif
