#include "BattleRoom.h"
#include "StringTable.h"
#include "BattleChat.h"
#include "Cache.h"
#include "ITabs.h"
#include "Prefs.h"
#include "MapImage.h"
#include "AddBotDialog.h"
#include "PopupMenu.h"

#include "log/Log.h"
#include "model/Model.h"

#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Tile.H>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>

static char const * PrefBattleRoomSplitV = "BattleRoomSplitV";

BattleRoom::BattleRoom(int x, int y, int w, int h, Model & model, Cache & cache, ITabs & iTabs):
    Fl_Tile(x,y,w,h),
    model_(model),
    cache_(cache),
    iTabs_(iTabs),
    battleId_(-1),
    lastRunning_(false)
{
    int const topH = h/2;
    top_ = new Fl_Group(x, y, w, topH);

    // header (text and buttons)
    //
    int const headerH = 48;
    int const rightW = 256; // 2 times image width
    topHeader_ = new Fl_Group(x, y, w-rightW, headerH);
    topHeader_ ->box(FL_FLAT_BOX);
    int headerX = x;

    int const headerTextW = w-rightW-250; // 250 pixels for buttons on right side of headerText
    headerText_ = new Fl_Multiline_Output(headerX, y, headerTextW, headerH);
    headerText_->box(FL_FLAT_BOX);

    headerX += headerTextW;
    specBtn_ = new Fl_Check_Button(headerX, y, 70, headerH/2, "Spec");
    readyBtn_ = new Fl_Check_Button(headerX, y+24, 70, headerH/2, "Ready");
    specBtn_->callback(BattleRoom::onSpec, this);
    readyBtn_->callback(BattleRoom::onReady, this);
    headerX += 70;

    // ally team choice
    teamBtn_ = new Fl_Choice(headerX+30, y, 50, headerH/2, "Ally");
    teamBtn_->align(FL_ALIGN_LEFT);
    teamBtn_->callback(BattleRoom::onAllyTeam, this);
    for (int i=0; i<16; ++i)
    {
        std::string str(boost::lexical_cast<std::string>(i+1));
        teamBtn_->add(str.c_str());
    }

    // add bot
    addBotBtn_ = new Fl_Button(headerX, y+24, 80, headerH/2, "Add AI...");
    addBotBtn_->callback(BattleRoom::onAddBot, this);

    headerX += 80;

    startBtn_ = new Fl_Button(headerX, y, 100, headerH/2, "Start Spring");
    startBtn_->callback(BattleRoom::onStart, this);
    startBtn_->deactivate();

    Fl_Button * btn;
    btn = new Fl_Button(headerX, y+24, 100, headerH/2, "Leave");
    btn->callback(BattleRoom::onLeave, this);

    topHeader_->resizable(headerText_);
    topHeader_->end();

    // map and info on right side
    //
    int const rightX = x + w - rightW;
    topRight_ = new Fl_Group(rightX, y, rightW, topH);

    mapImageBox_ = new MapImage(rightX, y, rightW/2, rightW/2);
    mapImageBox_->box(FL_FLAT_BOX);
    mapImageBox_->callback(BattleRoom::onMapImage, this);

    mapInfo_  = new Fl_Multiline_Output(rightX+rightW/2, y, rightW/2, rightW/2);
    mapInfo_->box(FL_FLAT_BOX);
    mapInfo_->color(FL_BACKGROUND2_COLOR);

    settings_  = new Fl_Box(rightX, y+rightW/2, rightW, topH-rightW/2);
    settings_->box(FL_FLAT_BOX);
    settings_->label("TODO");

    topRight_->resizable(settings_);
    topRight_->end();

    // player list
    //
    y += headerH;
    int const playerH = topH - headerH;

    playerList_ = new StringTable(x, y, w - rightW, playerH, "PlayerList",
            { "status", "sync", "name", "ally", "team", "rank", "country" });

    top_->resizable(playerList_);
    top_->end();

    // battle chat in bottom tile
    //
    y += playerH;
    battleChat_ = new BattleChat(x, y, w, h-topH, model);


    end();
    deactivate();

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

}

BattleRoom::~BattleRoom()
{
    prefs.set(PrefBattleRoomSplitV, battleChat_->y());
}

void BattleRoom::initTiles()
{
    int y;
    prefs.get(PrefBattleRoomSplitV, y, 0);
    if (y != 0)
    {
        position(0, battleChat_->y(), 0, y);
    }
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
    else
    {
        mapImageBox_->image(0);
        std::ostringstream oss;
        mapImageBox_->label("click to\ndownload map");
        mapImageBox_->activate();

        mapInfo_->value(0);
    }

    currentMapImage_ = battle.mapName();
}

void BattleRoom::joined(Battle const & battle)
{
    battleId_ = battle.id();
    playerList_->clear();
    activate();

    setHeaderText(battle);
    setMapImage(battle);

    lastRunning_ = battle.running();
    if (battle.running() && model_.me().battleStatus().sync() == 1)
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

    playerList_->sort();
    battleChat_->battleJoined(battle);
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

        User const & me = model_.me();
        if (battle.running() && me.battleStatus().sync() == 1 && !me.status().inGame())
        {
            if (lastRunning_ == false)
            {
                model_.startSpring();
                startBtn_->deactivate();
            }
            else
            {
                startBtn_->activate();
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
    if (battle.id() == battleId_)
    {
        playerList_->addRow(makeRow(user));
        battleChat_->addInfo(user.name() + " joined battle");
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
        }
    }
}

void BattleRoom::userChanged(User const & user)
{
    Battle const * b = user.joinedBattle();
    if ( b &&  b->id() == battleId_)
    {
        playerList_->updateRow(makeRow(user));
        User const & me = model_.me();
        if (user == me)
        {
            specBtn_->value(user.battleStatus().spectator());
            readyBtn_->value(user.battleStatus().ready());
            teamBtn_->value(user.battleStatus().allyTeam());
            mapImageBox_->setAlly( user.battleStatus().spectator() ? -1 : user.battleStatus().allyTeam());
            if (b->running() && me.battleStatus().sync() == 1 && !me.status().inGame())
            {
                startBtn_->activate();
            }
            else
            {
                startBtn_->deactivate();
            }
        }
    }
}

void BattleRoom::close()
{
    battleChat_->close();

    battleId_ = -1;

    mapImageBox_->image(0);
    mapImageBox_->label(0);
    currentMapImage_.clear();
    mapInfo_->value(0);

    headerText_->value("");

    playerList_->clear();
    startBtn_->deactivate();

    mapImageBox_->removeAllStartRects();

    deactivate();
}

std::string BattleRoom::statusString(User const & user)
{
    std::ostringstream oss;
    oss << (user.battleStatus().spectator() ? "S" : "")
        << (user.status().inGame() ? "G" : "")
        << (user.battleStatus().ready() ? "R" : "");
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

    return StringTableRow( user.name(),
        {
            statusString(user),
            syncString(user),
            user.name(),
            allyTeam.str(),
            team.str(),
            boost::lexical_cast<std::string>( user.status().rank() ),
            user.country()
        } );
}

StringTableRow BattleRoom::makeRow(Bot const & bot)
{
    boost::format allyTeam(bot.battleStatus().spectator() ? "s%2d" : "%2d");
    allyTeam % (bot.battleStatus().allyTeam() + 1);

    boost::format team(bot.battleStatus().spectator() ? "s%2d" : "%2d");
    team % (bot.battleStatus().team() + 1);

    return StringTableRow( bot.name() + ":" + bot.owner(),
        {
            "B",
            "",
            bot.name() + "," + bot.owner() + "," + bot.aiDll(),
            allyTeam.str(),
            team.str(),
            "",
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

    br->addBotDialog_->show(br->model_.getBattle(br->battleId_).modName());
}

void BattleRoom::onStart(Fl_Widget* w, void* data)
{
    BattleRoom * br = static_cast<BattleRoom*>(data);
    br->model_.startSpring();
    br->startBtn_->deactivate();
}

void BattleRoom::onLeave(Fl_Widget* w, void* data)
{
    BattleRoom * o = static_cast<BattleRoom*>(data);
    o->model_.leaveBattle();
    // o->close(); // TODO not needed here if we always get userLeftBattle(me, battle)
}

void BattleRoom::onMapImage(Fl_Widget* w, void* data)
{
    BattleRoom * o = static_cast<BattleRoom*>(data);
    o->handleOnMapImage();
}

void BattleRoom::botAdded(Bot const & bot)
{
    playerList_->addRow(makeRow(bot));
}

void BattleRoom::botChanged(Bot const & bot)
{
    playerList_->updateRow(makeRow(bot));
}

void BattleRoom::botRemoved(Bot const & bot)
{
    playerList_->removeRow(bot.name() + ":" + bot.owner());
}

void BattleRoom::springExit()
{
    // TODO this is probably not needed since model will set BattleStatus to NOT InGame when spring exit
}

void BattleRoom::setHeaderText(Battle const & battle)
{
    std::ostringstream oss;
    oss << battle.title() << " / " << battle.founder() << "\n"
        << battle.mapName() << "\n"
        << battle.modName() << "\n";
    headerText_->value(oss.str().c_str());
}

void BattleRoom::refresh()
{
    if (battleId_ != -1)
    {
        Battle const & b = model_.getBattle(battleId_);
        setMapImage(b);
    }
}

void BattleRoom::playerClicked(int rowIndex, int button)
{
    if (button == FL_RIGHT_MOUSE)
    {
        StringTableRow const & row = playerList_->getRow(static_cast<std::size_t>(rowIndex));

        PopupMenu menu;

        // try block will handle players, catch block will handle bots
        try
        {
            User const & user = model_.getUser(row.id_);
            menu.add("Open chat", 1);

            if (menu.size() > 0)
            {
                int const id = menu.show();
                switch (id)
                {
                case 1:
                    iTabs_.openPrivateChat(user.name());
                    break;
                }
            }
        }
        catch (std::invalid_argument const & e)
        {
            // bot, row.id_ is "botName:ownerName"
            std::vector<std::string> res;
            boost::algorithm::split(res, row.id_, boost::is_any_of(":"));
            assert(res.size() == 2);
            if (res[1] == model_.me().name())
            {
                menu.add("Remove", 1);
            }

            if (menu.size() > 0)
            {
                int const id = menu.show();
                switch (id)
                {
                case 1:
                    model_.removeBot(res[0]);
                    break;
                }
            }
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
                    if (mapImageBox_->image() == 0 && model_.downloadMap(mapName))
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
