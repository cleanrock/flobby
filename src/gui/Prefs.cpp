#include "Prefs.h"
#include "FlobbyDirs.h"

#include <cassert>

Fl_Preferences* prefs_ = 0;

void initPrefs()
{
    assert(prefs_ == 0);
    prefs_ = new Fl_Preferences(configDir().c_str(), "cleanrock", "flobby");
}

Fl_Preferences& prefs()
{
    assert(prefs_);
    return *prefs_;
}
