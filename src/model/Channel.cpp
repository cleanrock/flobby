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

    try
    {
        extractSentence(is, topic_);
    }
    catch (std::invalid_argument const & e)
    {
        // no topic, ignore
    }
}

Channel::~Channel()
{
}
