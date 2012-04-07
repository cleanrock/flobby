#pragma once

#include <model/Model.h>
#include "StringTable.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Preferences.H>
#include <boost/signal.hpp>
#include <string>


class ChannelsWindow: public Fl_Double_Window
{
public:
    ChannelsWindow(Model & model);
    virtual ~ChannelsWindow();

private:
    Model & model_;
    Fl_Preferences prefs_;

    StringTable * channelList_;
    bool channelsRetrieved_;

    static void callback(Fl_Widget*, void*);
    int handle(int event);
    void onChannels(Channels const & channels);
    StringTableRow makeRow(Channel const & channel);

    void channelClicked(int rowIndex, int button);
    void channelDoubleClicked(int rowIndex, int button);

};
