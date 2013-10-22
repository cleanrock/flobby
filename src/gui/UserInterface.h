#pragma once

#include <memory>
#include <string>
#include <deque>

// forwards
class Model;
class Cache;
class Battle;
class User;
class SpringDialog;
class LoginDialog;
class RegisterDialog;
class AgreementDialog;
class LoggingDialog;
class ProgressDialog;
class TextDialog;
class ChannelsWindow;
class MapsWindow;
class BattleList;
class BattleRoom;
class Tabs;
class ChatSettingsDialog;
class SoundSettingsDialog;
class FontSettingsDialog;

class Fl_Double_Window;
class Fl_Browser;
class Fl_Group;
class Fl_Menu_Bar;
class Fl_Tile;
class Fl_Widget;

class UserInterface
{
public:
    UserInterface(Model & model);
    virtual ~UserInterface();

    static void setupEarlySettings();

    int run(int argc, char** argv);

    void addCallbackEvent(void (*cb)(void*) /* Fl_Awake_Handler */, void *data);

private:
    static void setupLogging();

    Model & model_;
    std::unique_ptr<Cache> cache_;

    // map cache file generation variables
    enum GenType
    {
        GEN_INFO,
        GEN_MAP,
        GEN_METAL,
        GEN_HEIGHT
    };
    struct GenJob
    {
        std::string name_;
        GenType type_;
        GenJob(std::string const& name, GenType type): name_(name), type_(type) {}
    };
    std::deque<GenJob> genJobs_;
    std::size_t genJobsCount_;
    bool openMapsWindow_; // used for showing maps windows after map image files generation is done

    Fl_Double_Window * mainWindow_;
    Fl_Menu_Bar * menuBar_;

    ProgressDialog * progressDialog_;
    ChannelsWindow * channelsWindow_;
    MapsWindow * mapsWindow_;

    SpringDialog * springDialog_;
    LoginDialog * loginDialog_;
    RegisterDialog * registerDialog_;
    AgreementDialog * agreementDialog_;
    LoggingDialog * loggingDialog_;
    TextDialog * autoJoinChannelsDialog_;
    ChatSettingsDialog * chatSettingsDialog_;
    SoundSettingsDialog * soundSettingsDialog_;
    FontSettingsDialog * fontSettingsDialog_;

    Fl_Tile * tile_; // whole app window client area
    Fl_Tile * tileLeft_; // chat and battle list
    Tabs * tabs_;
    BattleList * battleList_;
    BattleRoom * battleRoom_;

    void loadAppIcon();
    void reloadMapsMods();
    void quit();

    // Model signal handlers
    void connected(bool connected);
    void loginResult(bool success, std::string const & info);
    void joinBattleFailed(std::string const & reason);
    void downloadDone(std::string const & name, bool success);

    // other signal handlers
    void autoJoinChannels(std::string const & text);
    void springProfileSet(std::string const & profile);

    // FLTK callbacks
    //
    static void menuLogin(Fl_Widget* w, void* d);
    static void menuDisconnect(Fl_Widget* w, void* d);
    static void menuRegister(Fl_Widget* w, void* d);
    static void onQuit(Fl_Widget* w, void* d);
    static void onTest(Fl_Widget* w, void* d);
    static void menuRefresh(Fl_Widget *w, void* d);
    static void menuGenerateCacheFiles(Fl_Widget *w, void* d);
    static void menuMaps(Fl_Widget *w, void* d);
    static void menuSpring(Fl_Widget *w, void* d);
    static void menuLogging(Fl_Widget *w, void* d);
    static void menuJoinChannel(Fl_Widget *w, void* d);
    static void menuChannels(Fl_Widget *w, void* d);
    static void menuBattleListFilter(Fl_Widget *w, void* d);
    static void mainWindowCallback(Fl_Widget * w, void * p); // used to stop Escape key from exiting the program
    static void menuChannelsAutoJoin(Fl_Widget *w, void* d);
    static void menuChatSettings(Fl_Widget *w, void* d);
    static void menuSoundSettings(Fl_Widget *w, void* d);
    static void menuFontSettings(Fl_Widget *w, void* d);
    static void checkAway(void* d);
    static void doGenJob(void* d);
    static void closeProgressDialog(void* d);

    void enableMenuItem(void(*cb)(Fl_Widget*, void*), bool enable);

};
