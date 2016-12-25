// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <vector>
#include <string>

std::pair<std::string, size_t> getLastWord(std::string const& text, std::size_t pos);

// returns true if text begins with needle, case-sensitive comparison
bool beginsC(std::string const& text, std::string const& needle);

// returns true if text begins with needle, case-insensitive comparison
bool beginsI(std::string const& text, std::string const& needle);

// returns true if text contain needle, case-insensitive comparison
bool containsI(std::string const& text, std::string const& needle);

std::string findMatch(std::vector<std::string> const& strings, std::string const& needle, std::string const& previousMatch = "");

std::pair<std::string, std::string> splitAtLast(char ch, std::string const& str);

// returns current local 24h time string, e.g. "17:59"
std::string getHourMinuteNow();

bool flOpenUri(std::string const& uri);
