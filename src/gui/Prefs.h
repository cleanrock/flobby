// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Preferences.H>

void initPrefs();
Fl_Preferences& prefs();

// prefs used by multiple classes
static char const * PrefFontSize = "FontSize";

char const * const PrefLogin = "Login";
char const * const PrefLoginHost = "Host";
char const * const PrefLoginPort = "Port";
char const * const PrefLoginUser = "User";
char const * const PrefLoginPassword = "Password";

char const * const PrefLogDebug = "LogDebug";
char const * const PrefLogChats = "LogChats";
