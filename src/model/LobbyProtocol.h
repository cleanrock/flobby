// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <iosfwd>
#include <string>

namespace LobbyProtocol
{

void extractWord(std::istream & is, std::string & ex);
void extractSentence(std::istream & is, std::string & ex);
void extractToNewline(std::istream & is, std::string & ex);
void skipSpaces(std::istream& is);

}; // namespace
