#include "Channel.h"
#include "LobbyProtocol.h"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>


Channel::Channel(std::istream & is) // channelName userCount [{topic}]
{
    using namespace LobbyProtocol;

    extractWord(is, name_);

    std::string ex;
    extractWord(is, ex);
    userCount_ = boost::lexical_cast<int>(ex);

    if (!is.eof())
    {
        extractSentence(is, topic_);
    }
}

Channel::~Channel()
{
}
