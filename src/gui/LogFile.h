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

    void log(std::string const & text);

private:
    std::string name_;
    std::ofstream ofs_;
};
