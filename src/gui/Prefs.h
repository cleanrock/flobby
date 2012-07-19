#pragma once

#include <FL/Fl_Preferences.H>

// prefs used by multiple classes
char const * const PrefLogin = "Login";
char const * const PrefLoginHost = "Host";
char const * const PrefLoginPort = "Port";
char const * const PrefLoginUser = "User";
char const * const PrefLoginPassword = "Password";

char const * const PrefLogDebug = "LogDebug";
char const * const PrefLogFilePath = "LogFilePath";
char const * const PrefLogChats = "LogChats";

extern Fl_Preferences prefs;
