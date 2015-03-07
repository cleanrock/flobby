// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "FlobbyDirs.h"
#include "log/Log.h"

#include <boost/filesystem.hpp>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <wordexp.h>

static std::string configDir_;
static std::string cacheDir_;

std::string wordExpand(std::string const& text)
{
    std::string result;
    wordexp_t we;
    int const r = wordexp(text.c_str(), &we, WRDE_NOCMD);
    if (r == EXIT_SUCCESS)
    {
        // only accept one result
        if (we.we_wordc == 1)
        {
            result = we.we_wordv[0];
        }
        else if (we.we_wordc == 0)
        {
            LOG(WARNING)<< "wordexp returned zero words, text='" << text << "'";
        }
        else
        {
            for (size_t i = 0; i < we.we_wordc; ++i)
            {
                LOG(WARNING)<< "word "<< i << " '" << we.we_wordv[i] << "'";
            }
            LOG(FATAL)<< "wordexp did not return one word ("<< (int)we.we_wordc << "), text='" << text << "'";
        }
        wordfree(&we);
    }
    else
    {
        LOG(FATAL)<< "wordexp failed ("<< r << "), text='" << text << "'";;
    }
    return result;
}

void initDirs(std::string const& dirOverride)
{
    using namespace boost::filesystem;

    if (dirOverride.empty())
    {
        configDir_ = wordExpand("${XDG_CONFIG_HOME:-~/.config}");
        cacheDir_  = wordExpand("${XDG_CACHE_HOME:-~/.cache}");

        // one more expand in case XDG var contain tilde, e.g. "~/flobby_zk"
        configDir_ = wordExpand(configDir_);
        cacheDir_ = wordExpand(cacheDir_);

        // append flobby subdir
        path configPath(configDir_);
        configPath /= "flobby/";
        configDir_ = configPath.string();

        path cachePath(cacheDir_);
        cachePath /= "flobby/";
        cacheDir_ = cachePath.string();
    }
    else
    {
        std::string dirExpanded = wordExpand(dirOverride);
        path dir(dirExpanded);

        if (dir.is_relative())
        {
            dir = absolute(dir);
        }
        configDir_ = dir.string();
        cacheDir_ = dir.string();
    }

    if (configDir_.empty()) LOG(FATAL)<< "config path empty";
    if (cacheDir_.empty()) LOG(FATAL)<< "cache path empty";

    // make sure paths end with slash
    if (configDir_.back() != '/') configDir_.append("/");
    if (cacheDir_.back() != '/') cacheDir_.append("/");

    LOG(INFO)<< "using config dir '"<< configDir_ << "'";
    LOG(INFO)<< "using cache dir '"<< cacheDir_ << "'";

    create_directories(configDir_);
    if (!is_directory(configDir_))
    {
        LOG(FATAL)<< "unable to create config dir";
    }
    create_directories(cacheDir_);
    if (!is_directory(cacheDir_))
    {
        LOG(FATAL)<< "unable to create cache dir";
    }

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
