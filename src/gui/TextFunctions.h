#pragma once

#include <vector>
#include <string>

// text must be none empty and end with a non-space, pair.second is position of last word
std::pair<std::string, size_t> getLastWord(std::string const& text, std::size_t pos);

// returns true if text begins with needle, case-sensitive comparison
bool beginsC(std::string const& text, std::string const& needle);

// returns true if text begins with needle, case-insensitive comparison
bool beginsI(std::string const& text, std::string const& needle);

// returns true if text contain needle, case-insensitive comparison
bool containsI(std::string const& text, std::string const& needle);

enum MatchResult
{
    MR_NO_MATCH = 0,
    MR_BEGINS_C,
    MR_BEGINS_I,
    MR_CONTAINS_I,
};

std::pair<MatchResult, std::string> findMatch(std::vector<std::string> const& strings, std::string const& needle);
