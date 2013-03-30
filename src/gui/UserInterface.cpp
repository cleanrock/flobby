#include "UserInterface.h"
#include "LogFile.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "AgreementDialog.h"
#include "LoggingDialog.h"
#include "ProgressDialog.h"
#include "ChannelsWindow.h"
#include "MapsWindow.h"
#include "BattleList.h"
#include "BattleInfo.h"
#include "BattleRoom.h"
#include "Prefs.h"
#include "Cache.h"
#include "Tabs.h"
#include "TextDialog.h"
#include "SpringDialog.h"
#include "ChatSettingsDialog.h"
#include "SoundSettingsDialog.h"
#include "FontSettingsDialog.h"

#include "log/Log.h"
#include "model/Model.h"

#include <X11/xpm.h>
#include <X11/extensions/scrnsaver.h>
#include <FL/x.H>
#include "icon.xpm.h"
#include <Magick++.h>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Group.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Shared_Image.H>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <cassert>

// Prefs
static char const * PrefAppWindowX = "AppWindowX";
static char const * PrefAppWindowY = "AppWindowY";
static char const * PrefAppWindowW  = "AppWindowW";
static char const * PrefAppWindowH = "AppWindowH";
static char const * PrefAppWindowSplitH = "AppWindowSplitH";
static char const * PrefLeftSplitV = "LeftSplitV";
static char const * PrefAutoJoinChannels = "AutoJoinChannels";


static XScreenSaverInfo* xScreenSaverInfo = 0;

UserInterface::UserInterface(Model & model) :
    model_(model),
    cache_(new Cache(model_))
{
    TextDisplay2::initTextStyles();

    Fl_File_Icon::load_system_icons();

    int const H = 1000;
    int const W = 1000;
    int const leftW = 400;
    int const rightW = 600; // hack to avoid problem with headertext in battle room
    int const mH = FL_NORMAL_SIZE*1.5;
    int const cH = H-mH;

    mainWindow_ = new Fl_Double_Window(W, H, "flobby");
    mainWindow_->user_data(this);
    mainWindow_->callback(UserInterface::mainWindowCallback);

    loadAppIcon();
    xScreenSaverInfo = XScreenSaverAllocInfo();

    Fl_Menu_Item menuitems[] = {
        { "&Server",              0, 0, 0, FL_SUBMENU },
            { "&Login...", FL_COMMAND +'l', (Fl_Callback *)&menuLogin, this },
            { "&Disconnect", 0, (Fl_Callback *)&menuDisconnect, this, FL_MENU_INACTIVE },
            { "&Register...", 0, (Fl_Callback *)&menuRegister, this },
            { "Re&name account...", 0, (Fl_Callback *)&menuRenameAccount, this },
            { "&Join channel...", FL_COMMAND +'j', (Fl_Callback *)&menuJoinChannel, this, FL_MENU_INACTIVE },
            { "&Channels...", FL_COMMAND +'h', (Fl_Callback *)&menuChannels, this, FL_MENU_INACTIVE },
//            { "&Test", FL_COMMAND +'t', (Fl_Callback *)&onTest, this }, // TODO disable before checkin
            { "E&xit", FL_COMMAND +'q', (Fl_Callback *)&onQuit, this },
            { 0 },
        { "Se&ttings",              0, 0, 0, FL_SUBMENU },
                { "&Spring...", FL_COMMAND +'s', (Fl_Callback *)&menuSpring, this },
                { "&Battle list filter...", FL_COMMAND +'b', (Fl_Callback *)&menuBattleListFilter, this },
                { "Channels to &auto-join...", 0, (Fl_Callback *)&menuChannelsAutoJoin, this },
                { "Soun&d...", 0, (Fl_Callback *)&menuSoundSettings, this },
                { "&Chat...", 0, (Fl_Callback *)&menuChatSettings, this },
                { "&Font ...", 0, (Fl_Callback *)&menuFontSettings, this },
                { "&Logging...", 0, (Fl_Callback *)&menuLogging, this },
                { 0 },
        { "&Other",              0, 0, 0, FL_SUBMENU },
            { "&Reload available games && maps", FL_COMMAND + 'r', (Fl_Callback *)&menuRefresh, this },
            { "&Generate missing cache files", 0, (Fl_Callback *)&menuGenerateCacheFiles, this },
            { "&Maps...", FL_COMMAND +'m', (Fl_Callback *)&menuMaps, this },
            { 0 },

        { 0 }
    };

    menuBar_ = new Fl_Menu_Bar(0, 0, W, mH);
    menuBar_->box(FL_FLAT_BOX);
    menuBar_->copy(menuitems);

    tile_ = new Fl_Tile(0, mH, W, cH);

    tileLeft_ = new Fl_Tile(0, mH, leftW, cH);
    int const tabsH = cH/2;
    tabs_ = new Tabs(0, mH, leftW, tabsH, model_);
    battleList_ = new BattleList(0, mH+tabsH, leftW, cH-tabsH, model_, *cache_);
    tileLeft_->end();

    battleRoom_ = new BattleRoom(leftW, mH, rightW, cH, model_, *cache_, *tabs_);

    tile_->end();

    mainWindow_->resizable(tile_);
    mainWindow_->end();

    channelsWindow_ = new ChannelsWindow(model_);
    mapsWindow_ = new MapsWindow(model_, *cache_);

    springDialog_ = new SpringDialog(model_);
    springDialog_->connectProfileSet(boost::bind(&UserInterface::springProfileSet, this, _1));
    loginDialog_ = new LoginDialog(model_);
    registerDialog_ = new RegisterDialog(model_);
    agreementDialog_ = new AgreementDialog(model_, *loginDialog_);
    loggingDialog_ = new LoggingDialog();
    progressDialog_ = new ProgressDialog();
    autoJoinChannelsDialog_ = new TextDialog("Channels to auto-join", "One channel per line");
    autoJoinChannelsDialog_->connectTextSave(boost::bind(&UserInterface::autoJoinChannels, this, _1));
    soundSettingsDialog_ = new SoundSettingsDialog();
    chatSettingsDialog_ = new ChatSettingsDialog();
    fontSettingsDialog_ = new FontSettingsDialog();
    tabs_->setChatSettingsDialog(chatSettingsDialog_); // ugly dependency injection


    // model signal handlers
    model.connectConnected( boost::bind(&UserInterface::connected, this, _1) );
    model.connectLoginResult( boost::bind(&UserInterface::loginResult, this, _1, _2) );
    model.connectJoinBattleFailed( boost::bind(&UserInterface::joinBattleFailed, this, _1) );
    model.connectDownloadDone( boost::bind(&UserInterface::downloadDone, this, _1, _2) );

    Magick::InitializeMagick(0);
}

UserInterface::~UserInterface()
{
    prefs.set(PrefAppWindowX, mainWindow_->x_root());
    prefs.set(PrefAppWindowY, mainWindow_->y_root());
    prefs.set(PrefAppWindowW, mainWindow_->w());
    prefs.set(PrefAppWindowH, mainWindow_->h());

    prefs.set(PrefAppWindowSplitH, battleRoom_->x());
    prefs.set(PrefLeftSplitV, battleList_->y());

    delete channelsWindow_;
    delete mapsWindow_;
    delete loginDialog_;
    delete mainWindow_;

    model_.disconnect();
}

void UserInterface::setupEarlySettings()
{
    setupLogging();
    FontSettingsDialog::setupFont();
}

void UserInterface::setupLogging()
{
    int logDebug;
    prefs.get(PrefLogDebug, logDebug, 0);

    char * logFilePath; // freed below
    prefs.get(PrefLogFilePath, logFilePath, "/tmp/flobby.log");

    if (logDebug != 0)
    {
        Log::minSeverity(DEBUG);
    }
    Log::logFile(logFilePath);

    ::free(logFilePath);

    int logChats;
    prefs.get(PrefLogChats, logChats, 1);
    LogFile::enable(logChats == 1 ? true : false);
}

int UserInterface::run(int argc, char** argv)
{
    {
        int x, y, w, h;
        prefs.get(PrefAppWindowX, x, 0);
        prefs.get(PrefAppWindowY, y, 0);
        prefs.get(PrefAppWindowW, w, 1000);
        prefs.get(PrefAppWindowH, h, 1000);
        mainWindow_->resize(x,y,w,h);

        prefs.get(PrefAppWindowSplitH, x, 0);
        if (x != 0)
        {
            tile_->position(battleRoom_->x(), 0, x, 0);
        }

        prefs.get(PrefLeftSplitV, y, 0);
        if (y != 0)
        {
            tileLeft_->position(0, battleList_->y(), 0, y);
        }
    }

    tabs_->initTiles();
    battleRoom_->initTiles();

    mainWindow_->show(argc, argv);

    // set paths to spring, unitsync and pr-downloader
    bool const pathsOk = springDialog_->setPaths();

    if (!pathsOk)
    {
        fl_alert("flobby is broken if paths to spring and unitsync are not set.\nSet them now or exit.");
    }

    if (pathsOk && loginDialog_->autoLogin())
    {
        loginDialog_->attemptLogin();
    }

    Fl::lock();
    return Fl::run();
}

void UserInterface::addCallbackEvent(Fl_Awake_Handler handler, void *data)
{
    assert(handler != 0);
    Fl::lock();
    const int awakeRes = Fl::awake(handler, data);
    assert(awakeRes == 0);
    Fl::unlock();
}

void UserInterface::menuLogin(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->loginDialog_->show();
}

void UserInterface::menuDisconnect(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->model_.disconnect();
}

void UserInterface::menuRegister(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->registerDialog_->show();
}

void UserInterface::menuRenameAccount(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    char const * str = fl_input("New name");
    if (str && ::strlen(str) > 0)
    {
        ui->model_.renameAccount(str);
    }
}

void UserInterface::menuJoinChannel(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    char const * str = fl_input("Join channel");
    if (str)
    {
        ui->model_.joinChannel(str);
    }
}

void UserInterface::menuChannels(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->channelsWindow_->show();
}

// TODO remove
void UserInterface::onTest(Fl_Widget *w, void* d)
{
    //UserInterface * ui = static_cast<UserInterface*>(d);
    //Model & m = ui->model_;

    // Sound::beep();
    // ui->sound_->play();

    // m.getModAIs("Zero-K v1.0.3.8");

#if 0
    auto maps = m.getMaps();
    unsigned char const * data = m.getMapImage(maps[0], 0);
    assert(data);
    cimg_library::CImg<unsigned char> img(1024, 1024, 1, 3);
    {
        int const siz = 1024*1024;
        unsigned char * d = img;
        unsigned char * r = d;
        unsigned char * g = r+siz;
        unsigned char * b = g+siz;

        for (int i=0; i < 1024*1024*3; i+=3)
        {
            *r = data[i+0]; ++r;
            *g = data[i+1]; ++g;
            *b = data[i+2]; ++b;
        }
    }
    // cimg_library::CImg<unsigned char> img("in.png");
    img.save_png("out.png", 3);
    // img.resize(128, 128, 1, 3); //, 6);
    img.resize_halfXY();
    img.save_png("out2.png", 3);
    img.resize_halfXY();
    img.save_png("out3.png", 3);
    img.resize_halfXY();
    img.save_png("out4.png", 3);
#endif

//    int width, height;
//    m.getMetalMap(maps[0], width, height);
//    m.getHeightMap(maps[0], width, height);
//    for (auto mapName : maps)
//    {
//        int width, height;
//        m.getMetalMapImage(mapName, width, height);
//    }

//    auto maps = m.getMaps();
//    MapInfo mi = m.getMapInfo(maps[0]);
//    std::ofstream ofs("mapinfo.txt");
//    std::cout << mi << std::endl;
//    ofs << mi;
//    ofs.close();
//    std::ifstream ifs("mapinfo.txt");
//    MapInfo mi2;
//    ifs >> mi2;
//    std::cout << mi << std::endl;
//    std::cout << (mi == mi2) << std::endl;


//    for (auto mapName : maps)
//    {
//        MapInfo mi = m.getMapInfo(mapName);
//        std::cout << mi;
//    }

    // write map image to png
    //
//    std::string const mapName = "Tabula-v4";
//    std::pair<uchar const *, int> res = ui->model_.getMapImage(mapName, 0);
//    if (res.first)
//    {
//        uchar const * p = res.first;
//        png::image< png::rgb_pixel > image(1024, 1024);
//        for (std::size_t y = 0; y < 1024; ++y)
//        {
//            for (std::size_t x = 0; x < 1024; ++x)
//            {
//                image[y][x] = png::rgb_pixel(p[0], p[1], p[2]);
//                p += 3;
//            }
//        }
//
//        image.write(mapName + ".png");
//    }


//    ui->channelsWindow_->show();

//    IControllerEvent & iControllerEvent = ui->model_;
//    iControllerEvent.message("SERVERMSG "
//            "line1 http://www.archlinux.org bla bla \n"
//            "line2 http://www.google.se\n"
//            "line3 bla bla ..."
//            );
//    iControllerEvent.message("SERVERMSG  ");
//    iControllerEvent.message("SAIDPRIVATE cavity hej hopp http://archlinux.org BBB");
//    iControllerEvent.message("SAIDPRIVATE cavity hej hopp http://archlinux.org CCC");

//    IChat * iChat = ui->tabs_;
//    iChat->openChannelChat("channel");
}

void UserInterface::connected(bool connected)
{
    enableMenuItem(UserInterface::menuDisconnect, connected);
    enableMenuItem(UserInterface::menuRegister, !connected);

    if (!connected)
    {
        enableMenuItem(UserInterface::menuLogin, true);
        enableMenuItem(UserInterface::menuJoinChannel, false);
        enableMenuItem(UserInterface::menuChannels, false);
        enableMenuItem(UserInterface::menuRenameAccount, false);
        channelsWindow_->hide();
        Fl::remove_timeout(checkAway);
    }
}

void UserInterface::loginResult(bool success, std::string const & info)
{
    enableMenuItem(UserInterface::menuLogin, !success);
    enableMenuItem(UserInterface::menuJoinChannel, success);
    enableMenuItem(UserInterface::menuChannels, success);
    enableMenuItem(UserInterface::menuRenameAccount, success);

    if (success)
    {
        char * val;
        prefs.get(PrefAutoJoinChannels, val, "");
        autoJoinChannels(val);
        ::free(val);

        checkAway(this);
    }
}

void UserInterface::joinBattleFailed(std::string const & reason)
{
    fl_alert("Join battle failed.\n%s", reason.c_str());
}

void UserInterface::onQuit(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->quit();
}

void UserInterface::menuRefresh(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->reloadMapsMods();
}

void UserInterface::menuGenerateCacheFiles(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    auto maps = ui->model_.getMaps();

    ui->progressDialog_->label("Generating cache files ...");
    ui->progressDialog_->show();

    int cnt = 0;
    for (auto const & map : maps)
    {
        float const percentage = 100*static_cast<float>(cnt)/maps.size();
        ui->progressDialog_->progress(percentage, map);
        try
        {
            Fl_Shared_Image * img;

            img = ui->cache_->getMapImage(map);
            if (img) img->release();

            img = ui->cache_->getMetalImage(map);
            if (img) img->release();

            img = ui->cache_->getHeightImage(map);
            if (img) img->release();

            ui->cache_->getMapInfo(map);
        }
        catch (std::exception const & e)
        {
            LOG(WARNING) << e.what();
        }
        ++cnt;
    }
    ui->progressDialog_->progress(100, "Done");
    ::usleep(500000); // TODO remove ?
    ui->progressDialog_->hide();
}

void UserInterface::menuMaps(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->mapsWindow_->show();
}

void UserInterface::menuBattleListFilter(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    ui->battleList_->showFilterDialog();
}

void UserInterface::menuSpring(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->springDialog_->show();
}

void UserInterface::menuChannelsAutoJoin(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    char * val;
    prefs.get(PrefAutoJoinChannels, val, "");
    std::string text(val);
    ::free(val);

    boost::replace_all(text, " ", "\n");

    ui->autoJoinChannelsDialog_->show(text.c_str());
}

void UserInterface::menuSoundSettings(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    ui->soundSettingsDialog_->show();
}

void UserInterface::menuChatSettings(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    ui->chatSettingsDialog_->show();
}

void UserInterface::menuFontSettings(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    ui->fontSettingsDialog_->show();
}

void UserInterface::menuLogging(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    ui->loggingDialog_->show();

}

void UserInterface::enableMenuItem(void(*cb)(Fl_Widget*, void*), bool enable)
{
    Fl_Menu_Item * mi = const_cast<Fl_Menu_Item *>(menuBar_->find_item(cb));
    if (mi == 0)
    {
        throw std::runtime_error("menu callback not found");
    }

    if (enable)
    {
        mi->activate();
    }
    else
    {
        mi->deactivate();
    }
}

void UserInterface::reloadMapsMods()
{
    model_.refresh();
    battleList_->refresh();
    battleRoom_->refresh();
}

void UserInterface::downloadDone(std::string const & name, bool success)
{
    reloadMapsMods();
}

void UserInterface::autoJoinChannels(std::string const & text)
{
    std::vector<std::string> channels;

    namespace ba = boost::algorithm;
    ba::split( channels, text, ba::is_any_of("\n "), ba::token_compress_on );

    std::ostringstream oss;
    for (auto & v : channels)
    {
        if (!v.empty())
        {
            oss << v << " ";
            model_.joinChannel(v);
        }
    }
    prefs.set(PrefAutoJoinChannels, oss.str().c_str());
}

void UserInterface::springProfileSet(std::string const & profile)
{
    std::string title = "flobby - " + profile;
    mainWindow_->copy_label(title.c_str());
}

void UserInterface::loadAppIcon()
{
    fl_open_display(); // needed if display has not been previously opened

    Pixmap p, mask;
    XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display),
                                     (char**)icon_xpm, &p, &mask, NULL);

//    Pixmap p = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display),
//                                     (char const *)icon_bits, icon_width, icon_height);

    mainWindow_->icon((const void *)p);
}

void UserInterface::mainWindowCallback(Fl_Widget * w, void * p)
{
    if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
    {
        return; // ignore Escape
    }
    UserInterface * ui = static_cast<UserInterface*>(p);
    ui->quit();
}

void UserInterface::quit()
{
    channelsWindow_->hide();
    mapsWindow_->hide();
    mainWindow_->hide();
}

void UserInterface::checkAway(void* d)
{
    if (fl_display && xScreenSaverInfo)
    {
        XScreenSaverQueryInfo(fl_display, DefaultRootWindow(fl_display), xScreenSaverInfo);
        UserInterface* ui = static_cast<UserInterface*>(d);

        if (xScreenSaverInfo->idle > 5*60*1000)
        {
            Fl::add_timeout(2.0, checkAway, d);
            ui->model_.meAway(true);
        }
        else
        {
            Fl::add_timeout(10.0, checkAway, d);
            ui->model_.meAway(false);
        }
    }
}
