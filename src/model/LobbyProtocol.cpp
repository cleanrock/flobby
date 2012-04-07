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
}



void extractSentence(std::istream & is, std::string & ex)
{
    std::getline(is, ex, '\t');
    if (is.fail())
    {
        throw std::invalid_argument("extractSentence failed");
    }
}

}; // namespace LobbyProtocol
