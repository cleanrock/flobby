// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Nightwatch.h"
#include <boost/regex.hpp>
//#include <iostream> // enable this and printing below when testing in unit test

NightwatchPm checkNightwatchPm(std::string const & msg)
{
    NightwatchPm result;

    //                                     ch     user    time    text
    const std::string pattern = "^\\!pm\\|(.*?)\\|(.*?)\\|(.*?)\\|(.*)";
    boost::regex regexPattern(pattern);
    boost::smatch what;
    bool const isMatchFound = boost::regex_match(msg, what, regexPattern);
    if (isMatchFound)
    {
        result.valid_ = true;
        assert(what.size() == 5);
        result.channel_ = what[1];
        result.user_ = what[2];
        result.time_ = what[3];
        result.text_ = what[4];

//        for (unsigned int i=0; i < what.size(); i++)
//        {
//            std::cout << "WHAT " << i << " " << what[i] << std::endl;
//        }
    }

    return result;
}
