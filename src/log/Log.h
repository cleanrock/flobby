// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <sstream>
#include <fstream>

class Log
{
public:
    enum Severity
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    Log(Severity sev, const char* loc, int line);
    ~Log();

    std::ostream & get()
    {
        return oss_;
    }

    static void logFile(std::string const & fileName); // must be called before log file creation to have effect
    static std::string const & logFile();
    static void minSeverity(Severity sev);
    static Severity minSeverity() { return minSev_; }

private:
    static std::ofstream ofs_;
    static std::string fileName_;
    static Severity minSev_;

    Severity sev_;
    std::ostringstream oss_;

    static char const * basename(char const * filePath);

};

/* TODO remove
struct NullStream : std::ostream
{
    NullStream() : std::ios(0), std::ostream(0) {}
};
*/

Log::Severity const DEBUG = Log::Debug;
Log::Severity const INFO = Log::Info;
Log::Severity const WARNING = Log::Warning;
Log::Severity const ERROR = Log::Error;
Log::Severity const FATAL = Log::Fatal;

class LogVoidify
{
public:
    LogVoidify() { }
    void operator&(std::ostream&) { } // This has to be an operator with a precedence lower than << but higher than ?:
};

#define LOG(sev) (sev < Log::minSeverity()) ? (void)0 : LogVoidify() & Log(sev, __FILE__, __LINE__).get()
#define LOG_IF(sev, cond) (sev < Log::minSeverity() || !(cond)) ? (void)0 : LogVoidify() & Log(sev, __FILE__, __LINE__).get()

/* TODO remove when i know i dont need DLOG
#ifndef NDEBUG
    #define DLOG(sev) LOG(sev)
    #define DLOG_IF(sev, test) LOG_IF(sev, test)
#else  // NDEBUG
    #define DLOG(sev) true ? (void)0 : LogVoidify() & LOG(sev)
    #define DLOG_IF(sev, test) (true || !(test)) ? (void)0 : LogVoidify() & LOG(sev)
#endif
*/
