// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "TextFunctions.h"

#include <boost/algorithm/string.hpp>
#include <cctype>
#include <ctime>

std::pair<std::string, size_t> getLastWord(std::string const& text, std::size_t pos)
{
    size_t posLastSpace = text.find_last_of(' ', pos);
    size_t posStart = (posLastSpace == std::string::npos) ? 0 : posLastSpace+1;

    return std::make_pair(text.substr(posStart, pos-posStart), posStart);
}

bool beginsC(std::string const& text, std::string const& needle)
{
    if (text.size() < needle.size())
    {
        return false;
    }

    for (size_t i=0; i<needle.size(); ++i)
    {
        if (text[i] != needle[i])
        {
            return false;
        }
    }
    return true;
}

bool beginsI(std::string const& text, std::string const& needle)
{
    if (text.size() < needle.size())
    {
        return false;
    }

    for (size_t i=0; i<needle.size(); ++i)
    {
        if (std::toupper(text[i]) != std::toupper(needle[i]))
        {
            return false;
        }
    }
    return true;
}

bool containsI(std::string const& text, std::string const& needle)
{
    typedef const boost::iterator_range<std::string::const_iterator> StringRange;

    if (boost::ifind_first(
            StringRange(text.begin(), text.end()),
            StringRange(needle.begin(), needle.end()) ))
    {
        return true;
    }
    return false;
}

std::pair<MatchResult, std::string> findMatch(std::vector<std::string> const& strings, std::string const& needle)
{
    std::pair<MatchResult, std::string> result;
    result.first = MR_NO_MATCH;

    if (needle.empty())
    {
        return result;
    }

    // Disabled beginsC for now.
    // I suspect it will just be confusing for the user.
    /*
    for (auto const& text : strings)
    {
        if (beginsC(text, needle))
        {
            result.first = MR_BEGINS_C;
            result.second = text;
            return result;
        }
    }
    */

    for (auto const& text : strings)
    {
        if (beginsI(text, needle))
        {
            result.first = MR_BEGINS_I;
            result.second = text;
            return result;
        }
    }

    for (auto const& text : strings)
    {
        if (containsI(text, needle))
        {
            result.first = MR_CONTAINS_I;
            result.second = text;
            return result;
        }
    }

    return result;
}

std::string getHourMinuteNow()
{
    char buf[8];
    std::time_t t = std::time(0);
    std::tm tm = *std::localtime(&t);
    std::strftime(buf, 8, "%H:%M", &tm);

    return std::string(buf);
}
