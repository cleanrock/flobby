// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "BattleRoom.h"
#include "StringTable.h"
#include "BattleChat.h"
#include "Cache.h"
#include "ITabs.h"
#include "Prefs.h"
#include "MapImage.h"
#include "AddBotDialog.h"
#include "PopupMenu.h"
#include "GameSettings.h"
#include "TextFunctions.h"
#include "SpringDialog.h"

#include "log/Log.h"
#include "model/Model.h"

#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Shared_Image.H>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <cassert>

static char const * PrefBattleRoomSplitV = "BattleRoomSplitV";

static char const* DL_GAME = "Download\nGame";
static char const* DL_ENGINE = "Download\nEngine";

BattleRoom::BattleRoom(int x, int y, int w, int h, Model & model, Cache & cache, ITabs & iTabs, SpringDialog& springDialog):
    Fl_Tile(x,y,w,h),
    model_(model),
    cache_(cache),
    iTabs_(iTabs),
    springDialog_(springDialog),
    battleId_(-1),
    lastRunning_(false)
{
    // limit split drag
    resizable( new Fl_Box(this->x(), this->y()+100, this->w(), this->h()-(100+100)) );

    int const topH = h/2;
    top_ = new Fl_Group(x, y, w, topH);

    // header (info text)
    //
    int headerH = FL_NORMAL_SIZE*3*1.3;
    headerH += (headerH & 1); // make sure headerH is even
    int const rightW = 256; // 2 times image width

    int const headerTextW = w-rightW;
    header_ = new Fl_Group(x, y, headerTextW, headerH);

    headerText_ = new Fl_Multiline_Output(x, y, headerTextW, headerH);
    headerText_->box(FL_THIN_DOWN_BOX);

    downloadGameBtn_ = new Fl_Button(0, 0, 2*headerH, headerH);
    downloadGameBtn_->callback(BattleRoom::onDownloadGame, this);
    header_->resizable(headerText_);
    header_->end();
    hideDownloadGameButton();

    // right side (buttons, map image and info, settings)
    //
    int const rightX = x + w - rightW;
    topRight_ = new Fl_Group(rightX, y, rightW, topH);
    topRight_->box(FL_FLAT_BOX); // needed to make right side redraw nicely when changing tiling

    // buttons (2 rows)
    int buttonX = x + headerTextW;
    int const buttonH = headerH/2; // safe, headerH is even
    // Spec and Ready
    specBtn_ = new Fl_Check_Button(buttonX, y, 70, buttonH, "Spec");
    readyBtn_ = new Fl_Check_Button(buttonX, y+buttonH, 70, buttonH, "Ready");
    specBtn_->callback(BattleRoom::onSpec, this);
    readyBtn_->callback(BattleRoom::onReady, this);
    buttonX += 70;
    // ally team choice
    teamBtn_ = new Fl_Choice(buttonX+30, y, 54, buttonH, "Ally");
    teamBtn_->align(FL_ALIGN_LEFT);
    teamBtn_->callback(BattleRoom::onAllyTeam, this);
    for (int i=0; i<16; ++i)
    {
        std::string str(boost::lexical_cast<std::string>(i+1));
        teamBtn_->add(str.c_str());
    }
    // add bot
    addBotBtn_ = new Fl_Button(buttonX, y+buttonH, 86, buttonH, "Add AI...");
    addBotBtn_->callback(BattleRoom::onAddBot, this);
    buttonX += 86;
    startBtn_ = new Fl_Button(buttonX, y, 100, buttonH, "Start Spring");
    startBtn_->callback(BattleRoom::onStart, this);
    startBtn_->deactivate();
    Fl_Button * btn;
    btn = new Fl_Button(buttonX, y+buttonH, 100, buttonH, "Leave");
    btn->callback(BattleRoom::onLeave, this);

    // map image
    mapImageBox_ = new MapImage(rightX, y+2*buttonH, rightW/2, rightW/2);
    mapImageBox_->box(FL_FLAT_BOX);
    mapImageBox_->callback(BattleRoom::onMapImage, this);

    // map info
    mapInfo_  = new Fl_Multiline_Output(rightX+rightW/2, y+2*buttonH, rightW/2, rightW/2);
    mapInfo_->box(FL_FLAT_BOX);
    mapInfo_->color(FL_BACKGROUND2_COLOR);

    // settings
    settings_  = new GameSettings(rightX, y+2*buttonH+rightW/2, rightW, topH-2*buttonH-rightW/2, model );

    topRight_->resizable(settings_);
    topRight_->end();

    // player list
    //
    y += headerH;
    int const playerH = topH - headerH;

    playerList_ = new StringTable(x, y, w - rightW, playerH, "PlayerList",
            { {"status",4}, {"sync",3}, {"name",10}, {"side",4}, {"ally",3}, {"team",4}, {"rank",3}, {"color",3}, {"country",4} }, 4 /* sort on ally by default */);

    top_->resizable(playerList_);
    top_->end();

    // battle chat in bottom tile
    //
    y += playerH;
    battleChat_ = new BattleChat(x, y, w, h-topH, model);


    end();
    top_->deactivate();

    addBotDialog_ = new AddBotDialog(model);

    // model signals
    model_.connectBattleJoined( boost::bind(&BattleRoom::joined, this, _1) );
    model_.connectBattleChanged( boost::bind(&BattleRoom::battleChanged, this, _1) );
    model_.connectBattleClosed( boost::bind(&BattleRoom::battleClosed, this, _1) );
    model_.connectUserJoinedBattle( boost::bind(&BattleRoom::userJoinedBattle, this, _1, _2) );
    model_.connectUserLeftBattle( boost::bind(&BattleRoom::userLeftBattle, this, _1, _2) );
    model_.connectUserChanged( boost::bind(&BattleRoom::userChanged, this, _1) );
    model_.connectBotAdded( boost::bind(&BattleRoom::botAdded, this, _1) );
    model_.connectBotChanged( boost::bind(&BattleRoom::botChanged, this, _1) );
    model_.connectBotRemoved( boost::bind(&BattleRoom::botRemoved, this, _1) );
    model_.connectAddStartRect( boost::bind(&BattleRoom::addStartRect, this, _1) );
    model_.connectRemoveStartRect( boost::bind(&BattleRoom::removeStartRect, this, _1) );
    model_.connectSpringExit( boost::bind(&BattleRoom::springExit, this) );
    model_.connectConnected( boost::bind(&BattleRoom::connected, this, _1) );

    playerList_->connectRowClicked( boost::bind(&BattleRoom::playerClicked, this, _1, _2) );
    playerList_->connectRowDoubleClicked( boost::bind(&BattleRoom::playerDoubleClicked, this, _1, _2) );

    battleChat_->getChatInput().connectComplete( boost::bind(&BattleRoom::onComplete, this, _1, _2, _3, _4) );

}

BattleRoom::~BattleRoom()
{
    prefs().set(PrefBattleRoomSplitV, battleChat_->y());
}

void BattleRoom::initTiles()
{
    int y;
    y = h()/2;
    prefs().get(PrefBattleRoomSplitV, y, y);
    position(0, battleChat_->y(), 0, y);
}

void BattleRoom::setMapImage(Battle const & battle)
{
    Fl_Image * image = cache_.getMapImage(battle.mapName());
    if (image)
    {
        mapImageBox_->label(0);
        mapImageBox_->image(image);
        mapImageBox_->activate();

        MapInfo const & mapInfo = cache_.getMapInfo(battle.mapName());
        std::ostringstream oss;
        oss << "Size: " << mapInfo.width_/512 << "x" << mapInfo.height_/512 << "\n"
            << "Wind: " << mapInfo.windMin_ << "-" << mapInfo.windMax_ << "\n"
            << "Tidal: " << mapInfo.tidalStrength_ << "\n"
            << "Gravity: " << mapInfo.gravity_ << "\n";

        mapInfo_->value(oss.str().c_str());
    }
    else if (!model_.getUnitSyncPath().empty())
    {
        mapImageBox_->image(0);
        std::string const msg = "click to\ndownload map\n" + battle.mapName();
        mapImageBox_->copy_label(msg.c_str());
        mapImageBox_->activate();

        mapInfo_->value(0);
    }
    else
    {
        // disable map download since we don't have unitsync yet
        mapImageBox_->image(0);
        mapImageBox_->label("");
        mapImageBox_->deactivate();

        mapInfo_->value(0);
    }

    currentMapImage_ = battle.mapName();
}

void BattleRoom::joined(Battle const & battle)
{
    battleId_ = battle.id();
    founder_ = battle.founder();

    playerList_->clear();
    top_->activate();

    // prio download of engine if unitsync is not loaded (we need unitsync to detect game properly)
    if ( !model_.gameExist(battle.modName()) && !model_.getUnitSyncPath().empty())
    {
        showDownloadGameButton(Model::DT_GAME);
        sideNames_.clear();
    }
    else if (springProfile_ != battle.engineVersion())
    {
        showDownloadGameButton(Model::DT_ENGINE);
    }
    else
    {
        hideDownloadGameButton();
        sideNames_ = model_.getModSideNames(battle.modName());
    }

    setMapImage(battle);

    lastRunning_ = battle.running();
    if (battle.running())
    {
        startBtn_->activate();
    }

    for (Battle::BattleUsers::value_type pair : battle.users())
    {
        assert(pair.second);
        User const & u = *pair.second;
        playerList_->addRow(makeRow(u));
    }

    for (Model::Bots::value_type pair : model_.getBots())
    {
        assert(pair.second);
        Bot const & b = *pair.second;
        playerList_->addRow(makeRow(b));
    }

    battleChat_->battleJoined(battle);

    balance_.clear();
    updateBalance();

    setHeaderText(battle);
}

void BattleRoom::battleChanged(const Battle & battle)
{
    if (battle.id() == battleId_)
    {
        if (currentMapImage_ != battle.mapName())
        {
            setMapImage(battle);
        }
        setHeaderText(battle);

        if (battle.running())
        {
            User const & me = model_.me();
            UserBattleStatus const& ubs = me.battleStatus();
            if (!me.status().inGame())
            {
                // start spring if game started and we are synced, except if we are spectator and not ready
                if ( lastRunning_ == false && ubs.sync() == 1 && ( !ubs.spectator() || (ubs.spectator() && ubs.ready()) ) )
                {
                    model_.startSpring();
                    startBtn_->deactivate();
                }
                else
                {
                    startBtn_->activate();
                }
            }
        }
        else
        {
            startBtn_->deactivate();
        }
        lastRunning_ = battle.running();
    }
}

void BattleRoom::battleClosed(const Battle & battle)
{
    if (battle.id() == battleId_)
    {
        close();
    }
}

void BattleRoom::userJoinedBattle(User const & user, const Battle & battle)
{
    // try to change spring profile if current profile do not match
    // a bit ugly to have switch here but we want to switch engine before calculating sync
    if (user == model_.me() && springProfile_ != battle.engineVersion())
    {
        springDialog_.setProfile(battle.engineVersion());
    }

    if (battle.id() == battleId_)
    {
        playerList_->addRow(makeRow(user));
        battleChat_->addInfo(user.name() + " joined battle");
        updateBalance();
    }
}

void BattleRoom::userLeftBattle(User const & user, const Battle & battle)
{
    if (battle.id() == battleId_)
    {
        battleChat_->addInfo(user.name() + " left battle");
        if (user == model_.me())
        {
            close();
        }
        else
        {
            playerList_->removeRow(user.name());
            updateBalance();
        }
    }
}

void BattleRoom::userChanged(User const & user)
{
    int const battleId = user.joinedBattle();
    if (battleId_ != -1 && battleId == battleId_)
    {
        playerList_->updateRow(makeRow(user));
        User const & me = model_.me();
        if (user == me)
        {
            specBtn_->value(user.battleStatus().spectator());
            readyBtn_->value(user.battleStatus().ready());
            teamBtn_->value(user.battleStatus().allyTeam());
            mapImageBox_->setAlly( user.battleStatus().spectator() ? -1 : user.battleStatus().allyTeam());
            Battle const& battle = model_.getBattle(battleId);
            if (battle.running() && !me.status().inGame())
            {
                startBtn_->activate();
            }
            else
            {
                startBtn_->deactivate();
            }
        }
        updateBalance();
    }
}

void BattleRoom::close()
{
    addBotDialog_->hide();

    battleChat_->close();

    battleId_ = -1;

    mapImageBox_->image(0);
    mapImageBox_->label(0);
    currentMapImage_.clear();
    mapInfo_->value(0);

    headerText_->value("");
    hideDownloadGameButton();

    playerList_->clear();
    startBtn_->deactivate();

    mapImageBox_->removeAllStartRects();

    settings_->clear();

    top_->deactivate();
}

std::string BattleRoom::statusString(User const & user)
{
    std::ostringstream oss;
    oss << (user.battleStatus().spectator() ? "S" : "")
        << (user.status().inGame() ? "G" : "")
        << (user.battleStatus().ready() ? "R" : "")
        << (user.status().away() ? "A" : "")
        << (user.name() == founder_ ? "H" : "");
    return oss.str();
}

std::string BattleRoom::syncString(User const & user)
{
    std::ostringstream oss;

    switch (user.battleStatus().sync())
    {
    case 0:
        oss << "?";
        break;
    case 2:
        oss << "no";
        break;
    }
    return oss.str();
}

StringTableRow BattleRoom::makeRow(User const & user)
{
    boost::format allyTeam(user.battleStatus().spectator() ? "s%2d" : "%2d");
    allyTeam % (user.battleStatus().allyTeam() + 1);

    boost::format team(user.battleStatus().spectator() ? "s%2d" : "%2d");
    team % (user.battleStatus().team() + 1);

    std::string side;
    if (user.battleStatus().side() < sideNames_.size())
    {
        side = sideNames_[user.battleStatus().side()];
    }
    else
    {
        side = boost::lexical_cast<std::string>( user.battleStatus().side() );
    }

    return StringTableRow( user.name(),
        {
            statusString(user),
            syncString(user),
            user.name(),
            side,
            allyTeam.str(),
            team.str(),
            boost::lexical_cast<std::string>( user.status().rank() ),
            boost::lexical_cast<std::string>( user.color() ),
            user.country()
        } );
}

StringTableRow BattleRoom::makeRow(Bot const & bot)
{
    boost::format allyTeam(bot.battleStatus().spectator() ? "s%2d" : "%2d");
    allyTeam % (bot.battleStatus().allyTeam() + 1);

    boost::format team(bot.battleStatus().spectator() ? "s%2d" : "%2d");
    team % (bot.battleStatus().team() + 1);

    std::string side;
    if (bot.battleStatus().side() < sideNames_.size())
    {
        side = sideNames_[bot.battleStatus().side()];
    }
    else
    {
        side = boost::lexical_cast<std::string>( bot.battleStatus().side() );
    }

    return StringTableRow( bot.name() + ":" + bot.owner(),
        {
            "B",
            "",
            bot.name() + "," + bot.owner() + "," + bot.aiDll(),
            side,
            allyTeam.str(),
            team.str(),
            "",
            boost::lexical_cast<std::string>( bot.color() ),
            ""
        } );
}

void BattleRoom::onSpec(Fl_Widget* w, void* data)
{
    Fl_Button * b = static_cast<Fl_Button*>(w);
    BattleRoom * o = static_cast<BattleRoom*>(data);
    o->model_.meSpec(b->value() == 1);
}

void BattleRoom::onReady(Fl_Widget* w, void* data)
{
    Fl_Button * b = static_cast<Fl_Button*>(w);
    BattleRoom * br = static_cast<BattleRoom*>(data);
    br->model_.meReady(b->value() == 1);
}

void BattleRoom::onAllyTeam(Fl_Widget* w, void* data)
{
    Fl_Choice * c = static_cast<Fl_Choice*>(w);
    BattleRoom * br = static_cast<BattleRoom*>(data);
    br->model_.meAllyTeam(c->value());
}

void BattleRoom::onAddBot(Fl_Widget* w, void* data)
{
    static_cast<void>(w);
    BattleRoom * br = static_cast<BattleRoom*>(data);

    std::string const modName = br->model_.getBattle(br->battleId_).modName();

    // try to find a unique bot name
    Model::Bots const & bots = br->model_.getBots();
    std::string botName = "AI_";
    for (int i=0; i<10; ++i)
    {
        std::string const botNameCandidate = botName + boost::lexical_cast<std::string>(i);
        if (bots.count(botNameCandidate) == 0)
        {
            botName = botNameCandidate;
            break;
        }
    }

    br->addBotDialog_->show(modName, botName);
}

void BattleRoom::onStart(Fl_Widget* w, void* data)
{
    BattleRoom * br = static_cast<BattleRoom*>(data);
    if (br->model_.isZeroK()) {
        br->model_.requestConnectSpring();
    }
    else {
        br->model_.startSpring();
    }
    br->startBtn_->deactivate();
}

void BattleRoom::onLeave(Fl_Widget* w, void* data)
{
    BattleRoom * o = static_cast<BattleRoom*>(data);
    o->model_.leaveBattle();
    // call to o->close(); not needed here if we always get userLeftBattle(me, battle)
}

void BattleRoom::onMapImage(Fl_Widget* w, void* data)
{
    BattleRoom * o = static_cast<BattleRoom*>(data);
    o->handleOnMapImage();
}

void BattleRoom::onDownloadGame(Fl_Widget* w, void* data)
{
    BattleRoom * o = static_cast<BattleRoom*>(data);
    o->handleOnDownloadGame();
}

void BattleRoom::botAdded(Bot const & bot)
{
    playerList_->addRow(makeRow(bot));
    updateBalance();
}

void BattleRoom::botChanged(Bot const & bot)
{
    playerList_->updateRow(makeRow(bot));
    updateBalance();
}

void BattleRoom::botRemoved(Bot const & bot)
{
    playerList_->removeRow(bot.name() + ":" + bot.owner());
    updateBalance();
}

void BattleRoom::springExit()
{
    // this is probably not needed since model will set BattleStatus to NOT InGame when spring exit
}

void BattleRoom::setHeaderText(Battle const & battle)
{
    std::ostringstream ossBalance;
    for (Balance::iterator it = balance_.begin(); it != balance_.end(); ++it)
    {
        ossBalance << it->second;
        if (it != --balance_.end())
        {
            ossBalance << 'v';
        }
    }

    std::ostringstream oss;
    oss << battle.title() << " / " << battle.founder() << " / " << battle.engineVersionLong() << "\n"
        << battle.mapName() << "  " << ossBalance.str() << "\n"
        << battle.modName();
    headerText_->value(oss.str().c_str());
}

void BattleRoom::refresh()
{
    if (battleId_ != -1)
    {
        Battle const & battle = model_.getBattle(battleId_);

        joined(battle);
    }
}

void BattleRoom::playerClicked(int rowIndex, int button)
{
    if (button == FL_RIGHT_MOUSE)
    {
        StringTableRow const & row = playerList_->getRow(static_cast<std::size_t>(rowIndex));

        // try block will handle players, catch block will handle bots
        try
        {
            User const & user = model_.getUser(row.id_);
            menuUser(user);
        }
        catch (std::invalid_argument const & e)
        {
            // bot, row.id_ is "botName:ownerName"
            std::vector<std::string> res;
            boost::algorithm::split(res, row.id_, boost::is_any_of(":"));
            assert(res.size() == 2);

            menuBot(res[0], res[1]);
        }
    }
}

void BattleRoom::playerDoubleClicked(int rowIndex, int button)
{
    if (button == FL_LEFT_MOUSE)
    {
        StringTableRow const & row = playerList_->getRow(static_cast<std::size_t>(rowIndex));

        try
        {
            User const & user = model_.getUser(row.id_);
            // player if we get here, i.e. getUser doesnt throw
            iTabs_.openPrivateChat(user.name());
        }
        catch (std::invalid_argument const & e)
        {
            // bot, remove bot
            std::vector<std::string> res;
            boost::algorithm::split(res, row.id_, boost::is_any_of(":"));
            assert(res.size() == 2);

            if (res[1] == model_.me().name())
            {
                model_.removeBot(res[0]);
            }
        }
    }
}

void BattleRoom::menuUser(User const& user)
{
    PopupMenu menu;

    menu.add(user.info());
    menu.add("Open chat", 1);

    if (user == model_.me())
    {
        for (int i=0; i<16; ++i)
        {
            std::ostringstream oss;
            oss << "Ally team/" << i+1;
            menu.add(oss.str(), 0x10+i, false);
        }
        for (int i=0; i<sideNames_.size(); ++i)
        {
            std::ostringstream oss;
            oss << "Side/" << sideNames_[i];
            menu.add(oss.str(), 0x20+i, false);
        }
    }

    std::string const zkAccountID = user.zkAccountID();
    if (!zkAccountID.empty()) {
        menu.add("Open user web page", 2);
    }

    if (menu.size() > 0)
    {
        int const id = menu.show();
        switch (id)
        {
        case 1:
            iTabs_.openPrivateChat(user.name());
            break;

        case 2: {
            std::string const link("http://zero-k.info/Users/Detail/" + zkAccountID);
            flOpenUri(link);
            break;
        }

        default:
            if (id >= 0x10 && id < 0x20)
            {
                int allyTeam = id - 0x10;
                model_.meAllyTeam(allyTeam);
            }
            else if (id >= 0x20 && id < 0x30)
            {
                int side = id - 0x20;
                model_.meSide(side);
            }
            break;
        }

    }
}

void BattleRoom::menuBot(std::string const& botName, std::string const& ownerName)
{
    PopupMenu menu;

    if (ownerName == model_.me().name())
    {
        menu.add("Remove", 1);

        for (int i=0; i<16; ++i)
        {
            std::ostringstream oss;
            oss << "Ally team/" << i+1;
            menu.add(oss.str(), 0x10+i, false);
        }
        for (int i=0; i<sideNames_.size(); ++i)
        {
            std::ostringstream oss;
            oss << "Side/" << sideNames_[i];
            menu.add(oss.str(), 0x20+i, false);
        }
    }

    if (menu.size() > 0)
    {
        int const id = menu.show();
        switch (id)
        {
        case 1:
            model_.removeBot(botName);
            break;

        default:
            if (id >= 0x10 && id < 0x20)
            {
                int allyTeam = id - 0x10;
                model_.botAllyTeam(botName, allyTeam);
            }
            else if (id >= 0x20 && id < 0x30)
            {
                int side = id - 0x20;
                model_.botSide(botName, side);
            }
            break;
        }
    }

}

void BattleRoom::addStartRect(StartRect const & startRect)
{
    mapImageBox_->addStartRect(startRect);
}

void BattleRoom::removeStartRect(int ally)
{
    mapImageBox_->removeStartRect(ally);
}

void BattleRoom::connected(bool connected)
{
    if (!connected)
    {
        close();
    }
}

void BattleRoom::handleOnMapImage()
{
    if (battleId_ == -1)
    {
        LOG(WARNING)<< "battleId_ == -1";
        return;
    }

    std::string const mapName = model_.getBattle(battleId_).mapName();
    if (mapName.empty())
    {
        LOG(WARNING)<< "mapName empty";
        return;
    }

    switch (Fl::event())
    {
        case FL_PUSH: // mouse button single click
            switch (Fl::event_button())
            {
                case FL_LEFT_MOUSE: // start download or display minimap
                {
                    if (mapImageBox_->image() == 0 && model_.downloadPr(mapName, Model::DT_MAP))
                    {
                        mapImageBox_->label("downloading...");
                        mapImageBox_->deactivate();
                    }
                    else
                    {
                        Fl_Image * image = cache_.getMapImage(mapName);
                        if (image && image != mapImageBox_->image())
                        {
                            mapImageBox_->image(image);
                            mapImageBox_->redraw();
                        }
                    }
                }
                break;
            }
            break;

        case FL_MOUSEWHEEL:
            Fl_Image * imageCurrent = mapImageBox_->image();
            if (imageCurrent != 0)
            {
                Fl_Image * imageHeight = cache_.getHeightImage(mapName);
                Fl_Image * imageMap = cache_.getMapImage(mapName);
                Fl_Image * imageMetal = cache_.getMetalImage(mapName);

                if (Fl::event_dy() < 0) // wheel up
                {
                    if (imageCurrent == imageMap)
                    {
                        mapImageBox_->image(imageHeight);
                        mapImageBox_->redraw();
                    }
                    else if (imageCurrent == imageMetal)
                    {
                        mapImageBox_->image(imageMap);
                        mapImageBox_->redraw();
                    }
                }
                else if (Fl::event_dy() > 0) // wheel down
                {
                    if (imageCurrent == imageMap)
                    {
                        mapImageBox_->image(imageMetal);
                        mapImageBox_->redraw();
                    }
                    else if (imageCurrent == imageHeight)
                    {
                        mapImageBox_->image(imageMap);
                        mapImageBox_->redraw();
                    }
                }
            }
            break;
    }
}

void BattleRoom::handleOnDownloadGame()
{
    if (battleId_ == -1)
    {
        LOG(WARNING)<< "battleId_ == -1";
        return;
    }

    std::string const btnText = downloadGameBtn_->label();

    Model::DownloadType downloadType;
    std::string downloadName;
    if (btnText == DL_GAME)
    {
        downloadType = Model::DT_GAME;
        downloadName = model_.getBattle(battleId_).modName();
    }
    else if (btnText == DL_ENGINE)
    {
        downloadType = Model::DT_ENGINE;
        // Make it possible to download development engines from other branches than develop.
        // Non-develop branch engines contain branch name in springname (see http://api.springfiles.com/).
        // I add a * to be able to download these, git hash should make it unique.
        downloadName = model_.getBattle(battleId_).engineVersion();
        if (downloadName.size() > 6)
        {
            downloadName += "*";
        }
    }
    else
    {
        LOG(WARNING)<< "unknown downloadType: '" << btnText << "'";
        return;
    }


    if (downloadName.empty())
    {
        LOG(WARNING)<< "downloadName empty";
        return;
    }

    if (model_.downloadPr(downloadName, downloadType))
    {
        downloadGameBtn_->label("Downloading...");
        downloadGameBtn_->deactivate();
        if (mapImageBox_->image() == 0)
        {
            mapImageBox_->deactivate();
        }
    }
}

void BattleRoom::hideDownloadGameButton()
{
    headerText_->size(header_->w(), header_->h());
    downloadGameBtn_->hide();
    header_->init_sizes();
}

void BattleRoom::showDownloadGameButton(Model::DownloadType downloadType)
{
    headerText_->size(header_->w() - 2*header_->h(), header_->h());
    downloadGameBtn_->resize(header_->x() + headerText_->w(), header_->y(), 2*header_->h(), header_->h());
    downloadGameBtn_->label( downloadType == Model::DT_GAME ? DL_GAME : DL_ENGINE);
    downloadGameBtn_->show();
    downloadGameBtn_->activate();
    header_->init_sizes();
}

void BattleRoom::onComplete(std::string const& text, std::size_t pos, std::string const& ignore, CompleteResult& result)
{
    auto const pairWordPos = getLastWord(text, pos);

    if (pairWordPos.first.empty())
    {
        LOG(DEBUG)<< "ignored trying to complete empty string";
        return;
    }

    std::vector<std::string> userNames;

    for (int i=0; i<playerList_->rows(); ++i)
    {
        StringTableRow const & row = playerList_->getRow(static_cast<std::size_t>(i));
        try
        {
            User const & user = model_.getUser(row.data_[2]);
            userNames.push_back(row.data_[2]);
        }
        catch (std::invalid_argument const & e)
        {
            // ignore bots
        }
    }

    auto const match = findMatch(userNames, pairWordPos.first, ignore);
    if (!match.empty())
    {
        result.match_ = match;
        result.newText_ = text.substr(0, pairWordPos.second) + match + text.substr(pos);
        result.newPos_ = pairWordPos.second + match.length();
    }
}

void BattleRoom::updateBalance()
{
    Balance balance;

    for (int i=0; i<playerList_->rows(); ++i)
    {
        StringTableRow const& row = playerList_->getRow(static_cast<std::size_t>(i));
        std::string str = row.data_[4]; // copy since we may remove initial space below
        if (str.empty())
        {
            LOG(WARNING)<< "unexpected empty ally team string";
            continue;
        }
        if (str[0] != 's') // only non-specs
        {
            if (str[0] == ' ')
            {
                str.erase(0, 1);
            }

            try
            {
                int allyTeam = boost::lexical_cast<int>(str);
                balance[allyTeam] += 1;
            }
            catch (boost::bad_lexical_cast const& ex)
            {
                LOG(WARNING)<< "unexpected allyTeam: " << str;
            }
        }
    }

    if (battleId_ != -1 && balance_ != balance)
    {
        balance_ = balance;
        setHeaderText(model_.getBattle(battleId_));
    }
}

