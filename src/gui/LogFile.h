// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <string>
#include <fstream>

class LogFile
{
public:
    LogFile(std::string const & name);
    virtual ~LogFile();

    static void init();
    static std::string const & dir();
    static bool enabled();
    static void enable(bool enable);

    std::string path();
    void log(std::string const & text);

    static void openLogFile(std::string const& path);

private:
    std::string name_;
    std::ofstream ofs_;
};
