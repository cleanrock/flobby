#include "BattleFilterDialog.h"
#include "Prefs.h"
#include "Sound.h"

#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


BattleFilterDialog::BattleFilterDialog():
    Fl_Window(400, 400, "Battle filter")
{
    set_modal();

    game_ = new Fl_Input(10, 30, 380, 30, "Game (e.g 'zero,nota')");
    game_->align(FL_ALIGN_TOP_LEFT);

    players_ = new Fl_Int_Input(10, 100, 380, 30, "Minimum players");
    players_->align(FL_ALIGN_TOP_LEFT);

    box_ = new Fl_Box(10, 170, 380, 30);
    box_->labelcolor(FL_RED);

    Fl_Return_Button * btn = new Fl_Return_Button(300, 350, 90, 30, "Set filter");
    btn->callback(BattleFilterDialog::callback, this);

    end();
}

BattleFilterDialog::~BattleFilterDialog()
{
}

void BattleFilterDialog::callback(Fl_Widget*, void *data)
{
    BattleFilterDialog * o = static_cast<BattleFilterDialog*>(data);
    o->onFilterSet();
}

void BattleFilterDialog::onFilterSet()
{
    int players = -1;
    try
    {
        players = boost::lexical_cast<int>(players_->value());
    }
    catch (boost::bad_lexical_cast & e)
    {
        // check below will display error
    }

    if (players < 0)
    {
        box_->label("Players must be a non-negative number");
        Sound::beep();
        return;
    }

    std::string gameFilter = game_->value();
    boost::trim(gameFilter);
    filterSetSignal_(gameFilter, players);
    box_->label(0);
    hide();
}

void BattleFilterDialog::show(std::string const & game, int players)
{
    game_->value(game.c_str());
    players_->value(boost::lexical_cast<std::string>(players).c_str());
    Fl_Window::show();
}

