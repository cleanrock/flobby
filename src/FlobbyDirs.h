#pragma once

#include <string>

void initDirs();

// return values below ends with '/'
std::string const& configDir();
std::string const& cacheDir();
