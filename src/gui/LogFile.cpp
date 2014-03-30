// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "LogFile.h"
#include "FlobbyDirs.h"

#include <boost/filesystem.hpp>
#include <stdexcept>
#include <ctime>

static std::string dir_;
static bool enabled_ = false;

LogFile::LogFile(std::string const & name):
    name_(name)
{
}

LogFile::~LogFile()
{
}

void LogFile::init()
{
    assert(dir_.empty());
    dir_ = cacheDir() + "log/";
    if (!boost::filesystem::is_directory(dir_.c_str()))
    {
        boost::filesystem::create_directories(dir_.c_str());
    }
}

std::string const & LogFile::dir()
{
    assert(!dir_.empty());
    return dir_;
}

void LogFile::log(std::string const & text)
{
    if (!enabled_) return;

    char buf[32];
    std::time_t t = std::time(0);
    std::tm tm = *std::localtime(&t);
    std::strftime(buf, 32, "%F %T", &tm);

    if (!ofs_.is_open())
    {
        std::string const fileName = dir() + name_ + ".log";
        ofs_.open(fileName, std::fstream::app);
        if (!ofs_.good())
        {
            throw std::runtime_error("failed to open log file: " + fileName);
        }
        ofs_ << "\nNEW LOG SESSION " << buf << std::endl;
    }
    ofs_ << buf << ": " << text << std::endl;
    if (!ofs_.good())
    {
        throw std::runtime_error("problem writing to log : " + name_);
    }
}

bool LogFile::enabled()
{
    return enabled_;
}

void LogFile::enable(bool enable)
{
    enabled_ = enable;
}
