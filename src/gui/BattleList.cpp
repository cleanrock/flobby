#include "BattleList.h"
#include "StringTable.h"
#include "BattleInfo.h"
#include "BattleFilterDialog.h"
#include "PopupMenu.h"
#include "Prefs.h"
#include "log/Log.h"

#include "model/Model.h"

#include <FL/Fl_Tile.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Menu_Item.H>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <cassert>

static char const * PrefBattleFilterGame = "BattleFilterGame";
static char const * PrefBattleFilterPlayers = "BattleFilterPlayers";


BattleList::BattleList(int x, int y, int w, int h, Model & model, Cache & cache): // TODO cache is only needed by BattleInfo
    Fl_Group(x, y, w, h),
    model_(model)
{
    int const h1 = h-128;
    battleList_ = new StringTable(x, y, w, h1, "BattleList",
            { "status", "title / host", "engine", "game", "map", "players" });

    battleInfo_ = new BattleInfo(x, y+h1, w, h-h1, model_, cache);

    resizable(battleList_);
    end();

    battleFilterDialog_ = new BattleFilterDialog();

    // model signals
    model_.connectConnected( boost::bind(&BattleList::connected, this, _1) );
    model_.connectLoginResult( boost::bind(&BattleList::loginResult, this, _1, _2) );
    model_.connectBattleOpened( boost::bind(&BattleList::battleOpened, this, _1) );
    model_.connectBattleChanged( boost::bind(&BattleList::battleChanged, this, _1) );
    model_.connectBattleClosed( boost::bind(&BattleList::battleClosed, this, _1) );
    model_.connectUserJoinedBattle( boost::bind(&BattleList::userJoinedLeftBattle, this, _1, _2) );
    model_.connectUserLeftBattle( boost::bind(&BattleList::userJoinedLeftBattle, this, _1, _2) );


    battleList_->connectSelectedRowChanged( boost::bind(&BattleList::battleListRowChanged, this, _1) );
    battleList_->connectRowClicked( boost::bind(&BattleList::battleListRowClicked, this, _1, _2) );
    battleList_->connectRowDoubleClicked( boost::bind(&BattleList::battleListRowDoubleClicked, this, _1, _2) );

    battleFilterDialog_->connectFilterSet( boost::bind(&BattleList::setFilter, this, _1, _2) );

    // read prefs
    char str[65];
    prefs.get(PrefBattleFilterGame, str, "", 64);
    filterGame_ = str;
    prefs.get(PrefBattleFilterPlayers, filterPlayers_, 0);

}

BattleList::~BattleList()
{
    prefs.set(PrefBattleFilterGame, filterGame_.c_str());
    prefs.set(PrefBattleFilterPlayers, filterPlayers_);
}

void BattleList::loginResult(bool success, std::string const & info)
{
    if (success)
    {
        std::vector<Battle const *> battles = model_.getBattles();

        for (auto b : battles)
        {
            assert(b);
            battleOpened(*b);
        }
        battleList_->sort();
    }
}

void BattleList::battleOpened(const Battle & battle)
{
    if (passesFilter(battle))
    {
        battleList_->addRow(makeRow(battle));
    }
}



void BattleList::battleChanged(const Battle & battle)
{
    if (passesFilter(battle))
    {
        StringTableRow row = makeRow(battle);
        try
        {
            battleList_->updateRow(row);
        }
        catch (std::runtime_error & e)
        {
            // battle was not in list, add it
            battleList_->addRow(row);
        }
    }
    else
    {
        try
        {
            battleList_->removeRow(boost::lexical_cast<std::string>(battle.id()));
        }
        catch (std::runtime_error & e)
        {
            // battle was not in list, ignore
        }
    }
}



void BattleList::battleClosed(const Battle & battle)
{
    try
    {
        battleList_->removeRow(boost::lexical_cast<std::string>(battle.id()));
    }
    catch (std::runtime_error & e)
    {
        // battle was not in list, ignore
    }
}

void BattleList::userJoinedLeftBattle(User const & user, const Battle & battle)
{
    static_cast<void>(user); // unused
    battleChanged(battle);
}

void BattleList::refresh()
{
    battleInfo_->refresh();
}

void BattleList::battleListRowChanged(int rowIndex)
{
    if (rowIndex >= 0)
    {
        StringTableRow const & row = battleList_->getRow(static_cast<std::size_t>(rowIndex));

        int const battleId = boost::lexical_cast<int>(row.id_);
        try
        {
            Battle const & battle = model_.getBattle(battleId);
            battleInfo_->battle(battle);
        } catch (std::exception const & e)
        {
            LOG(WARNING) << "exception in battleListRowChanged:" << e.what();
        }
    }
    else
    {
        battleInfo_->reset();
    }

}

void BattleList::battleListRowDoubleClicked(int rowIndex, int button)
{
    if (button == FL_LEFT_MOUSE)
    {
        StringTableRow const & row = battleList_->getRow(static_cast<std::size_t>(rowIndex));

        int const battleId = boost::lexical_cast<int>(row.id_);
        Battle const & battle = model_.getBattle(battleId);
        joinBattle(battle);
    }
}

void BattleList::battleListRowClicked(int rowIndex, int button)
{
    if (button == FL_RIGHT_MOUSE)
    {
        StringTableRow const & row = battleList_->getRow(static_cast<std::size_t>(rowIndex));

        int const battleId = boost::lexical_cast<int>(row.id_);
        Battle const & battle = model_.getBattle(battleId);

        PopupMenu menu;
        menu.add("Join", 1);
        int const id = menu.show();
        switch (id)
        {
        case 1:
            joinBattle(battle);
            break;
        }
    }
}

void BattleList::joinBattle(Battle const & battle)
{
    if (battle.locked()) return;

    if (battle.passworded())
    {
        char const * password = fl_input("Enter battle password");
        if (password != NULL)
        {
            model_.joinBattle(battle.id(), password);
        }
    }
    else
    {
        model_.joinBattle(battle.id());
    }
}

std::string BattleList::statusString(Battle const & battle)
{
    std::ostringstream oss;
    oss << (battle.passworded() ? "P" : "")
        << (battle.locked() ? "L" : "")
        << (battle.running() ? "G" : "");
    return oss.str();
}

StringTableRow BattleList::makeRow(Battle const & battle)
{
    boost::format players("%2d");
    players % battle.players();

    return StringTableRow( boost::lexical_cast<std::string>(battle.id()),
            { statusString(battle),
              battle.title() + " / " + battle.founder(),
              battle.engineVersion(),
              battle.modName(),
              battle.mapName(),
              players.str() } );
}

void BattleList::connected(bool connected)
{
    if (!connected)
    {
        battleList_->clear();
        battleInfo_->reset();
    }
}

void BattleList::setFilter(std::string const & game, int players)
{
    filterGame_ = game;
    filterPlayers_ = players;

    battleList_->clear();

    for (Battle const * b : model_.getBattles())
    {
        if (passesFilter(*b))
        {
            battleList_->addRow(makeRow(*b));
        }

    }
}

bool BattleList::passesFilter(Battle const & battle)
{
    if (battle.players() < filterPlayers_) return false;

    if (!filterGame_.empty())
    {
        std::vector<std::string> games;
        boost::algorithm::split(games, filterGame_, boost::is_any_of(","));

        int matches = 0;
        for (auto& game : games)
        {
            boost::trim(game);

            if (game.empty())
            {
                // ignore empty game filters
                matches += 1;
            }
            else if ( !boost::algorithm::ifind_first(battle.modName(), game).empty() )
            {
                matches += 1;
            }
        }
        if (matches == 0)
        {
            return false;
        }
    }

    return true;
}

void BattleList::showFilterDialog()
{
    battleFilterDialog_->show(filterGame_, filterPlayers_);
}
