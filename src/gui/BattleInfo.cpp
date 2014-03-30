// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "BattleInfo.h"
#include "StringTable.h"
#include "Cache.h"

#include "model/Model.h"

#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Shared_Image.H>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <sstream>

BattleInfo::BattleInfo(int x, int y, int w, int h, Model & model, Cache & cache):
    Fl_Group(x,y,w,h),
    model_(model),
    cache_(cache),
    battleId_(-1)
{
    // heigth should be 128
    assert(h == 128);

    // TODO remove ? box(FL_FLAT_BOX);

    mapImageBox_ = new Fl_Box(x,y, h, h);
    mapImageBox_->box(FL_FLAT_BOX);

    x += 128;
    headerText_ = new Fl_Multiline_Output(x, y, w-h, h);
    headerText_->box(FL_THIN_DOWN_BOX);
    headerText_->wrap(1);

    resizable(headerText_);
    end();

    // model signal handlers
    model_.connectBattleChanged( boost::bind(&BattleInfo::battleChanged, this, _1) );
    model_.connectBattleClosed( boost::bind(&BattleInfo::battleClosed, this, _1) );
    model_.connectUserJoinedBattle( boost::bind(&BattleInfo::userJoinedBattle, this, _1, _2) );
    model_.connectUserLeftBattle( boost::bind(&BattleInfo::userLeftBattle, this, _1, _2) );
    model_.connectUserChanged( boost::bind(&BattleInfo::userChanged, this, _1) );

    reset();
}

BattleInfo::~BattleInfo()
{
}

void BattleInfo::setMapImage(Battle const & battle)
{
    Fl_Image * image = cache_.getMapImage(battle.mapName());
    if (image)
    {
        mapImageBox_->label(0);
        mapImageBox_->image(image);
        currentMapImage_ = battle.mapName();
    }
    else
    {
        mapImageBox_->image(0);
        mapImageBox_->label("map\nnot\nfound");
        currentMapImage_.clear();
    }
}

void BattleInfo::battle(Battle const & battle)
{
    battleId_ = battle.id();

    setMapImage(battle);
    setHeaderText(battle);
}

void BattleInfo::reset()
{
    battleId_ = -1;
    mapImageBox_->image(0);
    mapImageBox_->label(0);
    currentMapImage_.clear();
    headerText_->value("");
}

void BattleInfo::onJoin(Fl_Widget* w, void* data)
{
    BattleInfo * bi = static_cast<BattleInfo*>(data);
    static_cast<void>(bi);
    // TODO be able to join from here ?
}

void BattleInfo::setHeaderText(Battle const & battle)
{
    std::ostringstream oss;
    oss << battle.title() << " / " << battle.founder() << " / " << battle.engineVersion() <<"\n"
        << battle.mapName() << "\n"
        << battle.modName() << "\n"
        << "Users:";

    for (Battle::BattleUsers::value_type pair : battle.users())
    {
        assert(pair.second);
        User const & u = *pair.second;
        oss << "  " << u.name();
    }

    headerText_->value(oss.str().c_str());
}

void BattleInfo::refresh()
{
    if (battleId_ != -1)
    {
        Battle const & b = model_.getBattle(battleId_);
        setMapImage(b);
    }
}

void BattleInfo::battleClosed(const Battle & battle)
{
    if (battle.id() == battleId_)
    {
        reset();
    }
}

void BattleInfo::battleChanged(const Battle & battle)
{
    if (battle.id() == battleId_)
    {
        if (currentMapImage_ != battle.mapName())
        {
            setMapImage(battle);
        }
        setHeaderText(battle);
    }
}

void BattleInfo::userJoinedBattle(User const & user, const Battle & battle)
{
    if (battle.id() == battleId_)
    {
        setHeaderText(battle);
    }
}

void BattleInfo::userLeftBattle(User const & user, const Battle & battle)
{
    if (battle.id() == battleId_)
    {
        setHeaderText(battle);
    }
}

void BattleInfo::userChanged(User const & user)
{
    int const battleId = user.joinedBattle();
    if ( battleId == battleId_)
    {
        // TODO ?
    }
}
