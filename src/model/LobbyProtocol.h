#pragma once

#include <iosfwd>
#include <string>

namespace LobbyProtocol
{

void extractWord(std::istream & is, std::string & ex);
void extractSentence(std::istream & is, std::string & ex);

}; // namespace LobbyProtocol
