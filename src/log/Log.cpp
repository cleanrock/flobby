// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Log.h"

#include <mutex>
#include <iostream>
#include <cstring>

std::ostringstream Log::earlyLogs_;
std::ofstream Log::ofs_;
std::string Log::fileName_;
Log::Severity Log::minSev_ = Log::Info;

static std::mutex m;

static char const * const severityStrings[] =
{
  "D-", "I-", "W-", "E-", "F-"
};

Log::Log(Severity sev, char const * loc, int line)
: sev_(sev)
{
    // time stamp
    char buf[16];
    std::time_t t = std::time(0);
    tm_ = *std::localtime(&t);
    std::strftime(buf, 16, "%H:%M:%S", &tm_);

    oss_ << severityStrings[sev_] << buf << "-" << basename(loc) << ":" << line << "] ";
}

Log::~Log()
{
    std::lock_guard<std::mutex> lock(m);

    if (sev_ >= Log::Info)
    {
        std::cout << oss_.str() << std::endl;
    }

    if (!ofs_.is_open())
    {
        if (0 == earlyLogs_.tellp())
        {
            char bufFirst[32];
            std::strftime(bufFirst, 32, "%F %T %z", &tm_);
            earlyLogs_ << "NEW LOG SESSION " << bufFirst << std::endl;
        }

        if (fileName_.empty())
        {
            earlyLogs_ << oss_.str() << std::endl;
        }
        else
        {
            ofs_.open(fileName_);
            if (!ofs_.good())
            {
                std::cout << "failed to open log file: " << fileName_ << std::endl;
                std::abort();
            }
            ofs_ << earlyLogs_.str();
            ofs_ << oss_.str() << std::endl;
        }
    }
    else
    {
        ofs_ << oss_.str() << std::endl;
    }

    if (sev_ == Fatal)
    {
        std::abort();
    }
}

void Log::logFile(std::string const & fileName)
{
    fileName_ = fileName;
}

std::string const & Log::logFile()
{
    return fileName_;
}

void Log::minSeverity(Severity sev)
{
    minSev_ = sev;
}

char const * Log::basename(char const * filePath)
{
    char const * pos = std::strrchr(filePath, '/');
    return pos ? pos+1 : filePath;
}
