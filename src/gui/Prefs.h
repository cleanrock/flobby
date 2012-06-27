#pragma once

#include <FL/Fl_Preferences.H>

// log prefs (used in main and LoggingDialog)
char const * const PrefLogDebug = "LogDebug";
char const * const PrefLogFilePath = "LogFilePath";
char const * const PrefLogChats = "LogChats";

extern Fl_Preferences prefs;
