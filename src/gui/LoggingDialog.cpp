#include "LoggingDialog.h"
#include "log/Log.h"
#include "LogFile.h"
#include "Prefs.h"

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


LoggingDialog::LoggingDialog():
    Fl_Window(400, 400, "Logging")
{
    set_modal();

    logDebug_ = new Fl_Check_Button(10, 30, 380, 30, "Log debug messages");

    logFilePath_ = new Fl_File_Input(10, 90, 380, 40, "Log file (change takes effect on restart)");
    logFilePath_->align(FL_ALIGN_TOP_LEFT);

    logChats_ = new Fl_Check_Button(10, 160, 380, 30, "Chat history (stored in ~/.spring/flobby/log/)");

    Fl_Return_Button * btn = new Fl_Return_Button(300, 360, 90, 30, "Save");
    btn->callback(LoggingDialog::callbackApply, this);

    end();
}

LoggingDialog::~LoggingDialog()
{
}

void LoggingDialog::show()
{
    // these are set in main at start
    logDebug_->value(Log::minSeverity() == Log::Debug ? 1 : 0);
    logFilePath_->value(Log::logFile().c_str());
    logChats_->value(LogFile::enabled() ? 1 : 0);

    Fl_Window::show();
}

void LoggingDialog::callbackApply(Fl_Widget*, void *data)
{
    LoggingDialog * o = static_cast<LoggingDialog*>(data);

    prefs.set(PrefLogDebug, o->logDebug_->value());
    Log::minSeverity(o->logDebug_->value() == 1 ? Log::Debug : Log::Info);

    prefs.set(PrefLogChats, o->logChats_->value());
    LogFile::enable(o->logChats_->value() == 1 ? true : false);

    // sanity check of log file path
    namespace fs = boost::filesystem;
    fs::path logPath(o->logFilePath_->value());
    if (!logPath.is_absolute())
    {
        o->badFilePath("Log file must be an absolute path.\n" + logPath.string());
        return;
    }

    if (fs::is_directory(logPath))
    {
        o->badFilePath("Log file is a directory.\n" + logPath.string());
        return;
    }

    if (!fs::is_directory(logPath.parent_path()))
    {
        o->badFilePath("Log file directory do not exist.\n" + logPath.string());
        return;
    }

    if (logPath.filename().string().size() <= 2)
    {
        o->badFilePath("Log file name is too short.\n" + logPath.string());
        return;
    }

    Log::logFile(logPath.string());
    prefs.set(PrefLogFilePath, logPath.string().c_str());

    o->hide();
}

void LoggingDialog::badFilePath(std::string const & msg)
{
    fl_alert("%s", msg.c_str());

    // restore text field
    logFilePath_->value(Log::logFile().c_str());
}
