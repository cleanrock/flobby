#pragma once

#include <FL/Fl_Window.H>
#include <boost/signal.hpp>
#include <string>

class Fl_Input;
class Fl_Int_Input;
class Fl_Box;

class BattleFilterDialog: public Fl_Window
{
public:
    BattleFilterDialog();
    virtual ~BattleFilterDialog();

    void show(std::string const & game, int players);

    // signals
    //
    typedef boost::signal<void (std::string const & game, int players)> FilterSetSignal;
    boost::signals::connection connectFilterSet(FilterSetSignal::slot_type subscriber)
    { return filterSetSignal_.connect(subscriber); }

private:
    Fl_Input * game_;
    Fl_Int_Input * players_;
    Fl_Box * box_;
    FilterSetSignal filterSetSignal_;

    static void callback(Fl_Widget*, void*);
    void onFilterSet();
};
