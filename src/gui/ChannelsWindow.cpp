// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "ChannelsWindow.h"
#include "Prefs.h"

#include "model/Model.h"

#include <FL/Fl.H>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

static char const * PrefWindowX = "WindowX";
static char const * PrefWindowY = "WindowY";
static char const * PrefWindowW  = "WindowW";
static char const * PrefWindowH = "WindowH";

ChannelsWindow::ChannelsWindow(Model & model):
    Fl_Double_Window(100, 100, "Channels"),
    model_(model),
    prefs_(prefs(), label()),
    channelsRetrieved_(false)
{
    channelList_ = new StringTable(0, 0, 100, 100, "ChannelList",
            { "name", "users", "topic" });

    model_.connectChannels( boost::bind(&ChannelsWindow::onChannels, this, _1) );

    resizable(channelList_);
    end();

    int x, y, w, h;
    prefs_.get(PrefWindowX, x, 0);
    prefs_.get(PrefWindowY, y, 0);
    prefs_.get(PrefWindowW, w, 600);
    prefs_.get(PrefWindowH, h, 600);
    resize(x,y,w,h);

    channelList_->connectRowClicked( boost::bind(&ChannelsWindow::channelClicked, this, _1, _2) );
    channelList_->connectRowDoubleClicked( boost::bind(&ChannelsWindow::channelDoubleClicked, this, _1, _2) );
}

ChannelsWindow::~ChannelsWindow()
{
    prefs_.set(PrefWindowX, x_root());
    prefs_.set(PrefWindowY, y_root());
    prefs_.set(PrefWindowW, w());
    prefs_.set(PrefWindowH, h());
}

void ChannelsWindow::callback(Fl_Widget*, void *data)
{
    // TODO not needed atm, remove
    // ChannelsWindow * o = static_cast<ChannelsWindow*>(data);
}

void ChannelsWindow::onChannels(Channels const & channels)
{
    channelList_->clear();
    for (Channel const & channel : channels)
    {
        channelList_->addRow(makeRow(channel));
    }
}

StringTableRow ChannelsWindow::makeRow(Channel const & channel)
{
    boost::format userCount("%4d");
    userCount % channel.userCount();

    return StringTableRow( channel.name(),
        {
            channel.name(),
            userCount.str(),
            channel.topic()
        } );
}

int ChannelsWindow::handle(int event)
{
    switch (event)
    {
    case FL_SHOW:
        if (!channelsRetrieved_)
        {
            model_.getChannels();
            channelsRetrieved_ = true;
        }
        break;
    }

    return Fl_Double_Window::handle(event);
}

void ChannelsWindow::channelClicked(int rowIndex, int button)
{
    if (button == FL_RIGHT_MOUSE)
    {
        StringTableRow const & row = channelList_->getRow(static_cast<std::size_t>(rowIndex));

        std::string const & channelName = row.id_;

        // TODO context menu
    }
}

void ChannelsWindow::channelDoubleClicked(int rowIndex, int button)
{
    if (button == FL_LEFT_MOUSE)
    {
        StringTableRow const & row = channelList_->getRow(static_cast<std::size_t>(rowIndex));

        std::string const & channelName = row.id_;

        model_.joinChannel(channelName);
    }
}
