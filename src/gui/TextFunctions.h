#pragma once

#include <string>

// text must be none empty and end with a non-space, pair.second is position of last word
std::pair<std::string, size_t> getLastWord(std::string const& text);

// returns true if text contain needle, case-insensitive comparison
bool containsI(std::string const& text, std::string const& needle);
