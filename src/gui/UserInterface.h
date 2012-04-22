#pragma once

#include "Cache.h"
#include <FL/Fl.H>
#include <memory>

// forwards
class Model;
class Battle;
class User;
class LoginDialog;
class ProgressDialog;
class ChannelsWindow;
class BattleList;
class BattleRoom;
class ChatTabs;

class Fl_Double_Window;
class Fl_Browser;
class Fl_Group;
class Fl_Menu_Bar;
class Fl_Tile;

class UserInterface
{
public:
    UserInterface(Model & model);
    virtual ~UserInterface();

    int run(int argc, char** argv);

    void addCallbackEvent(Fl_Awake_Handler handler, void *data);

private:
    Model & model_;
    Cache cache_;

    Fl_Double_Window * mainWindow_;
    Fl_Menu_Bar * menuBar_;
    LoginDialog * loginDialog_;
    ProgressDialog * progressDialog_;
    ChannelsWindow * channelsWindow_;

    Fl_Tile * tile_; // whole app window client area
    Fl_Tile * tileLeft_; // chat and battle list
    ChatTabs * chat_;
    BattleList * battleList_;
    BattleRoom * battleRoom_;

    void reloadMapsMods();

    // Model signal handlers
    void connected(bool connected);
    void loginResult(bool success, std::string const & info);
    void joinBattleFailed(std::string const & reason);
    void downloadDone(std::string const & name);

    // FLTK callbacks
    //
    static void menuLogin(Fl_Widget* w, void* d);
    static void menuDisconnect(Fl_Widget* w, void* d);
    static void onQuit(Fl_Widget* w, void* d);
    static void onTest(Fl_Widget* w, void* d);
    static void menuRefresh(Fl_Widget *w, void* d);
    static void menuGenerateCacheFiles(Fl_Widget *w, void* d);
    static void menuSpringPath(Fl_Widget *w, void* d);
    static void menuUnitSyncPath(Fl_Widget *w, void* d);
    static void menuChannels(Fl_Widget *w, void* d);
    static void menuBattleListFilter(Fl_Widget *w, void* d);

    void enableMenuItem(void(*cb)(Fl_Widget*, void*), bool enable);

};
