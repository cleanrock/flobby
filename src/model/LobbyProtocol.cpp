// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "LobbyProtocol.h"

#include <stdexcept>
#include <istream>

namespace LobbyProtocol
{

void extractWord(std::istream & is, std::string & ex)
{
    std::getline(is, ex, ' ');
    if (is.fail())
    {
        throw std::invalid_argument("extractWord failed");
    }

    // consume extra spaces, fix for zk uberserver sending 'TASServer 1.3.1.12  * 8201 0')
    skipSpaces(is);
}



void extractSentence(std::istream & is, std::string & ex)
{
    std::getline(is, ex, '\t');
    if (is.fail() && !is.eof())
    {
        throw std::invalid_argument("extractSentence failed");
    }
}

void skipSpaces(std::istream & is)
{
    while (is.peek() == ' ')
    {
        is.ignore(1);
    }
}

}; // namespace LobbyProtocol
