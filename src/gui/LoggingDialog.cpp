// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "LoggingDialog.h"
#include "log/Log.h"
#include "LogFile.h"
#include "Prefs.h"

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Return_Button.H>


LoggingDialog::LoggingDialog():
    Fl_Window(400, 400, "Logging")
{
    set_modal();

    flobbyLogPath_ = new Fl_Output(10, 30, 380, 30, "Flobby log");
    flobbyLogPath_->align(FL_ALIGN_TOP_LEFT);
    flobbyLogPath_->value(Log::logFile().c_str());

    logDebug_ = new Fl_Check_Button(10, 60, 380, 30, "Log debug messages");

    chatLogDir_ = new Fl_Output(10, 130, 380, 30, "Chat history directory");
    chatLogDir_->align(FL_ALIGN_TOP_LEFT);
    chatLogDir_->value(LogFile::dir().c_str());

    logChats_ = new Fl_Check_Button(10, 160, 380, 30, "Log chats");

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
    logChats_->value(LogFile::enabled() ? 1 : 0);

    Fl_Window::show();
}

void LoggingDialog::callbackApply(Fl_Widget*, void *data)
{
    LoggingDialog * o = static_cast<LoggingDialog*>(data);

    prefs().set(PrefLogDebug, o->logDebug_->value());
    Log::minSeverity(o->logDebug_->value() == 1 ? Log::Debug : Log::Info);

    prefs().set(PrefLogChats, o->logChats_->value());
    LogFile::enable(o->logChats_->value() == 1 ? true : false);

    o->hide();
}
