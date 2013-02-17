#include "TextFunctions.h"

#include <boost/algorithm/string.hpp>

std::pair<std::string, size_t> getLastWord(std::string const& text, std::size_t pos)
{
    size_t posLastSpace = text.find_last_of(' ', pos);
    size_t posStart = (posLastSpace == std::string::npos) ? 0 : posLastSpace+1;

    return std::make_pair(text.substr(posStart, pos-posStart), posStart);
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
