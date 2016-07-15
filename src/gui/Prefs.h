// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Preferences.H>
#include <boost/signals2/signal.hpp>


void initPrefs();
Fl_Preferences& prefs();

// prefs used by multiple classes
char const * const PrefFontSize = "FontSize";
char const * const PrefServerMessagesSplitH = "ServerMessagesSplitH";

char const * const PrefLogin = "Login";
char const * const PrefLoginHost = "Host";
char const * const PrefLoginPort = "Port";
char const * const PrefLoginUser = "User";
char const * const PrefLoginPassword = "Password";

char const * const PrefLogDebug = "LogDebug";
char const * const PrefLogChats = "LogChats";

struct BattleChatSettings
{
    bool showVoteLineMessages;
    void save();
};
BattleChatSettings& battleChatSettings();
typedef boost::signals2::signal<void (void)> BattleChatSettingsChangedSignal;
boost::signals2::connection connectBattleChatSettingsChanged(BattleChatSettingsChangedSignal::slot_type subscriber);

