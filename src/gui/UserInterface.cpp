#include "UserInterface.h"
#include "LoginDialog.h"
#include "ProgressDialog.h"
#include "ChannelsWindow.h"
#include "BattleList.h"
#include "BattleInfo.h"
#include "BattleRoom.h"
#include "Prefs.h"
#include "MyImage.h"
#include "Cache.h"
#include "Tabs.h"
#include "logging.h"

#include <X11/xpm.h>
#include <FL/x.H>
#include "icon.xpm.h"
//#include "icon.xbm.h"

#include "model/Model.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Group.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <cassert>

// Prefs
static char const * PrefAppWindowX = "AppWindowX";
static char const * PrefAppWindowY = "AppWindowY";
static char const * PrefAppWindowW  = "AppWindowW";
static char const * PrefAppWindowH = "AppWindowH";
static char const * PrefAppWindowSplitH = "AppWindowSplitH";
static char const * PrefLeftSplitV = "LeftSplitV";
static char const * PrefSpringPath = "SpringPath";
static char const * PrefUnitSyncPath = "UnitSyncPath";

UserInterface::UserInterface(Model & model) :
    model_(model),
    cache_(model_)
{
    FL_NORMAL_SIZE = 12; // TODO ??

    Fl_File_Icon::load_system_icons();

    int const H = 1000;
    int const W = 1000;
    int const leftW = 400;
    int const rightW = 600; // hack to avoid problem with headertext in battle room
    int const mH = 24;
    int const cH = H-mH;

    mainWindow_ = new Fl_Double_Window(W, H, "flobby");
    mainWindow_->user_data((void*) (this));

    loadAppIcon();

    Fl_Menu_Item menuitems[] = {
        { "&Server",              0, 0, 0, FL_SUBMENU },
            { "&Login...", FL_COMMAND +'l', (Fl_Callback *)&menuLogin, this },
            { "&Disconnect", 0, (Fl_Callback *)&menuDisconnect, this, FL_MENU_INACTIVE },
            { "&Channels...", FL_COMMAND +'h', (Fl_Callback *)&menuChannels, this, FL_MENU_INACTIVE },
//            { "&Test", FL_COMMAND +'t', (Fl_Callback *)&onTest, this }, // TODO remove
            { "E&xit", FL_COMMAND +'q', (Fl_Callback *)&onQuit, this },
            { 0 },
        { "Se&ttings",              0, 0, 0, FL_SUBMENU },
                { "&Spring path...", 0, (Fl_Callback *)&menuSpringPath, this },
                { "&UnitSync path...", 0, (Fl_Callback *)&menuUnitSyncPath, this },
                { 0 },
        { "&View",              0, 0, 0, FL_SUBMENU },
            { "&Reload available games && maps", FL_COMMAND + 'r', (Fl_Callback *)&menuRefresh, this },
            { "&Generate missing cache files", 0, (Fl_Callback *)&menuGenerateCacheFiles, this },
            { "&Battle list filter...", 0, (Fl_Callback *)&menuBattleListFilter, this },
            { 0 },

        { 0 }
    };

    menuBar_ = new Fl_Menu_Bar(0, 0, W, mH);
    menuBar_->copy(menuitems);

    tile_ = new Fl_Tile(0, mH, W, cH);

    tileLeft_ = new Fl_Tile(0, mH, leftW, cH);
    tabs_ = new Tabs(0, mH, leftW, cH/2, model_);
    battleList_ = new BattleList(0, mH+cH/2, leftW, cH/2, model_, cache_);
    tileLeft_->end();

    battleRoom_ = new BattleRoom(leftW, mH, rightW, cH, model_, cache_, *tabs_);

    tile_->end();

    mainWindow_->resizable(tile_);
    mainWindow_->end();

    loginDialog_ = new LoginDialog(model_);

    progressDialog_ = new ProgressDialog();

    channelsWindow_ = new ChannelsWindow(model_);

    // model signal handlers
    model.connectConnected( boost::bind(&UserInterface::connected, this, _1) );
    model.connectLoginResult( boost::bind(&UserInterface::loginResult, this, _1, _2) );
    model.connectJoinBattleFailed( boost::bind(&UserInterface::joinBattleFailed, this, _1) );
    model.connectDownloadDone( boost::bind(&UserInterface::downloadDone, this, _1) );

    MyImage::registerHandler();
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
    delete loginDialog_;
    delete mainWindow_;

    model_.disconnect();
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

    // set Spring and UnitSync paths
    {
        char buf[257];
        std::string str;

        prefs.get(PrefSpringPath, buf, "", 256);
        str = buf;
        if (!boost::filesystem::is_regular_file(str))
        {
            menuSpringPath(0, this);
        }
        else
        {
            model_.setSpringPath(str);
        }

        prefs.get(PrefUnitSyncPath, buf, "", 256);
        str = buf;
        if (!boost::filesystem::is_regular_file(str))
        {
            menuUnitSyncPath(0, this);
        }
        else
        {
            model_.setUnitSyncPath(str);
        }

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

void UserInterface::menuChannels(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->channelsWindow_->show();
}

// TODO remove
void UserInterface::onTest(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    Model & m = ui->model_;

    //m.getModAIs("Zero-K v1.0.3.8");
    // fl_beep();

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
//    iControllerEvent.message("SAIDPRIVATE cavity hej hopp http://archlinux.org AAA");
//    iControllerEvent.message("SAIDPRIVATE cavity hej hopp http://archlinux.org BBB");
//    iControllerEvent.message("SAIDPRIVATE cavity hej hopp http://archlinux.org CCC");

//    IChat * iChat = ui->tabs_;
//    iChat->openChannelChat("channel");
}

void UserInterface::connected(bool connected)
{
    enableMenuItem(UserInterface::menuDisconnect, connected ? true : false);

    if (!connected)
    {
        enableMenuItem(UserInterface::menuLogin, true);
        enableMenuItem(UserInterface::menuChannels, false);
        channelsWindow_->hide();
    }
}

void UserInterface::loginResult(bool success, std::string const & info)
{
    enableMenuItem(UserInterface::menuLogin, success ? false : true);
    enableMenuItem(UserInterface::menuChannels, success ? true : false);

    // TODO do something here ???
    if (!success)
    {
    }
    else
    {
    }
}

void UserInterface::joinBattleFailed(std::string const & reason)
{
    fl_alert("Join battle failed.\n%s", reason.c_str());
}

void UserInterface::onQuit(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);
    ui->channelsWindow_->hide();
    ui->mainWindow_->hide();
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
    for (auto map : maps)
    {
        float const percentage = 100*static_cast<float>(cnt)/maps.size();
        ui->progressDialog_->progress(percentage, map);
        try
        {
            ui->cache_.getMapImage(map);
            ui->cache_.getMapInfo(map);
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

void UserInterface::menuBattleListFilter(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    ui->battleList_->showFilterDialog();
}

void UserInterface::menuSpringPath(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    Fl_Native_File_Chooser fc;
    fc.options(Fl_Native_File_Chooser::NO_OPTIONS);
    fc.title("Select spring executable");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);

    char buf[257];
    if (prefs.get(PrefSpringPath, buf, "/usr/bin/spring", 256))
    {
        fc.preset_file(buf);
    }
    else
    {
        fc.preset_file(buf);
//        fc.directory(buf);
    }

    int res = fc.show();
    if (res == 0 && boost::filesystem::is_regular_file(fc.filename()))
    {
        ui->model_.setSpringPath(fc.filename());
        prefs.set(PrefSpringPath, fc.filename());
    }
    if ( !boost::filesystem::is_regular_file(ui->model_.getSpringPath()) )
    {
        fl_alert("flobby is broken without spring.");
    }
}

void UserInterface::menuUnitSyncPath(Fl_Widget *w, void* d)
{
    UserInterface * ui = static_cast<UserInterface*>(d);

    Fl_Native_File_Chooser fc;
    fc.options(Fl_Native_File_Chooser::NO_OPTIONS);
    fc.title("Select UnitSync library");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);

    char buf[257];
    if (prefs.get(PrefUnitSyncPath, buf, "/usr/lib/libunitsync.so", 256))
    {
        fc.preset_file(buf);
    }
    else
    {
        fc.preset_file(buf);
//        fc.directory(buf);
    }

    int res = fc.show();
    if (res == 0 && boost::filesystem::is_regular_file(fc.filename()))
    {
        ui->model_.setUnitSyncPath(fc.filename());
        prefs.set(PrefUnitSyncPath, fc.filename());
    }
    if ( !boost::filesystem::is_regular_file(ui->model_.getUnitSyncPath()) )
    {
        fl_alert("flobby is broken without UnitSync.");
    }
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

void UserInterface::downloadDone(std::string const & name)
{
    reloadMapsMods();
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
