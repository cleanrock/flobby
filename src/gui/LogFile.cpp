#include "LogFile.h"

#include <boost/filesystem.hpp>
#include <stdexcept>
#include <ctime>

std::string const LogFile::dir_ = "flobby/log/";
bool LogFile::enabled_ = false;

LogFile::LogFile(std::string const & name):
    name_(name)
{
}

LogFile::~LogFile()
{
}

std::string const & LogFile::dir()
{
    if (!boost::filesystem::is_directory(dir_.c_str()))
    {
        boost::filesystem::create_directories(dir_.c_str());
    }
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
        std::string const fileName = dir() + "flobby_" + name_ + ".log";
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
