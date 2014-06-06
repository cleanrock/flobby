// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "FlobbyDirs.h"

#include <boost/filesystem.hpp>
#include <cstring>
#include <cstdlib>
#include <cassert>

static std::string configDir_;
static std::string cacheDir_;

void initDirs()
{
    const char* home = ::getenv("HOME");
    assert(home);

    const char* configHome = ::getenv("XDG_CONFIG_HOME");
    if (configHome && ::strlen(configHome) > 0)
    {
        configDir_ = configHome;
    }
    else
    {
        configDir_ = home;
        configDir_ += "/.config";
    }
    configDir_ += "/flobby/";
    boost::filesystem::create_directories(configDir_);

    const char* cacheHome = ::getenv("XDG_CACHE_HOME");
    if (cacheHome && ::strlen(cacheHome) > 0)
    {
        cacheDir_ = cacheHome;
    }
    else
    {
        cacheDir_ = home;
        cacheDir_ += "/.cache";
    }
    cacheDir_ += "/flobby/";
    boost::filesystem::create_directories(cacheDir_);
}

std::string const& configDir()
{
    assert(!configDir_.empty());
    return configDir_;
}

std::string const& cacheDir()
{
    assert(!cacheDir_.empty());
    return cacheDir_;
}
