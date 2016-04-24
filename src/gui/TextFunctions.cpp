// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "TextFunctions.h"
#include "log/Log.h"
#include <FL/filename.H> // fl_open_uri
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <ctime>

// return pair with word before pos (up to pos) and pos of word start
// e.g. "asd gh|f ert" would return ["gh", 4]
// text must be none empty and end with a non-spac
std::pair<std::string, size_t> getLastWord(std::string const& text, std::size_t pos)
{
    // find beginning of last word (last space before pos/cursor)
    size_t posLastSpace = text.find_last_of(' ', pos-1);
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

std::string findMatch(std::vector<std::string> const& strings, std::string const& needle, std::string const& previousMatch)
{
    std::string result;

    if (needle.empty()) {
        return result;
    }

    std::vector<std::string> matches;

    // Disabled for now, I suspect it will be confusing for the user.
    /*
    // add entries that start with needle, case-sensitive
    for (auto const& text : strings)
    {
        if (beginsC(text, needle))
        {
            matches.push_back(text);
        }
    }
    */

    // add entries that start with needle, case-insensitive
    for (auto const& text : strings) {
        if (beginsI(text, needle)) {
            matches.push_back(text);
        }
    }

    // add entries that contain needle, case-insensitive
    for (auto const& text : strings) {
        if (containsI(text, needle)) {
            matches.push_back(text);
        }
    }

    if (!matches.empty()) {
        if (previousMatch.empty()) {
            result = matches.front();
        } else {
            // take next match, take first if previousMatch is the last
            auto it = std::find(matches.cbegin(), matches.cend(), previousMatch);
            if (it != matches.end()) {
                if ((it+1) == matches.end()) {
                    result = matches.front();
                } else {
                    result = *(++it);
                }
            }
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

bool flOpenUri(std::string const& uri)
{
    LOG(DEBUG) << "uri: '" << uri << "'";

    bool result = false;
    char msg[512];
    int const res = fl_open_uri(uri.c_str(), msg, sizeof(msg));
    if (res == 1) {
        LOG(DEBUG)<< "fl_open_uri success: " << msg;
        result = true;
    }
    else { // 0
        LOG(WARNING)<< "fl_open_uri(" << uri << ") failed: " << msg;
    }
    return result;
}
