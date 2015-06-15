// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Prefs.h"
#include "FlobbyDirs.h"

#include <cassert>

Fl_Preferences* prefs_ = nullptr;

// BattleChatSettings
//
static char const * const PrefBattleChatShowVoteLineMessages = "BattleChatShowVoteLineMessages";
static BattleChatSettings battleChatSettings_;
static BattleChatSettingsChangedSignal battleChatSettingsChangedSignal_;

void BattleChatSettings::save()
{
    prefs().set(PrefBattleChatShowVoteLineMessages, showVoteLineMessages);

    battleChatSettingsChangedSignal_();
}

BattleChatSettings& battleChatSettings()
{
    return battleChatSettings_;
}

boost::signals2::connection connectBattleChatSettingsChanged(BattleChatSettingsChangedSignal::slot_type subscriber)
{
    return battleChatSettingsChangedSignal_.connect(subscriber);
}


void initPrefs()
{
    assert(prefs_ == 0);
    prefs_ = new Fl_Preferences(configDir().c_str(), "cleanrock", "flobby");

    int val;

    prefs().get(PrefBattleChatShowVoteLineMessages, val, 0);
    battleChatSettings_.showVoteLineMessages = val;
}

Fl_Preferences& prefs()
{
    assert(prefs_);
    return *prefs_;
}
