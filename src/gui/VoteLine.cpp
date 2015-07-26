// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "VoteLine.h"
#include "TextFunctions.h"

#include "log/Log.h"
#include "model/Model.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/filename.H>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp> // <regex> in gcc 4.7 still "empty"
#include <sstream>

VoteLine::VoteLine(int x, int y, int w, int h, Model & model):
    Fl_Group(x, y, w, h),
    model_(model)
//    votesYes_(0),
//    votesNo_(0),
//    votesTotal_(0)
{
    box(FL_NO_BOX);

    int const btnWidth = h*1.2;

    text_ = new Fl_Box(x, y, w-2*btnWidth, h);
    text_->box(FL_THIN_DOWN_BOX);
    text_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    yes_ = new Fl_Button(x + w - 2*btnWidth, y, btnWidth, h, "Yes");
    no_ = new Fl_Button(x + w - 1*btnWidth, y, btnWidth, h, "No");

    yes_->callback(VoteLine::onYes, this);
    no_->callback(VoteLine::onNo, this);

    resizable(text_);
    end();

//    makeVotesString();
}

VoteLine::~VoteLine()
{
}

void VoteLine::onYes(Fl_Widget * w, void * data)
{
    VoteLine * o = static_cast<VoteLine*>(data);

    o->model_.sayBattle("!y");
}

void VoteLine::onNo(Fl_Widget * w, void * data)
{
    VoteLine * o = static_cast<VoteLine*>(data);

    o->model_.sayBattle("!n");
}

std::string VoteLine::checkSpringieVote(std::string const & msg)
{
    std::string voteText;

    if (msg.find("Poll: ") == 0)
    {
        voteText = msg.substr(6);
        if (msg.find("[END:") == std::string::npos)
        {
            activate();
        }
        else
        {
            deactivate();
        }
    }

    return voteText;
}

std::string VoteLine::checkSpadsVote(std::string const & msg)
{
    std::string voteText;

    std::vector<std::string> const enablers =
    {
        "called a vote for command",
        "Vote in progress:",
    };

    for (std::string const & text : enablers)
    {
        if (std::string::npos != msg.find(text))
        {
            activate();
            voteText = msg;
            return voteText;
        }
    }

    const std::vector<std::string> disablers =
    {
        "Vote for command",
        "Vote cancelled",
    };

    for (const std::string& text : disablers)
    {
        if (std::string::npos != msg.find(text))
        {
            deactivate();
            voteText = msg;
            return voteText;
        }
    }

    return voteText;
}

void VoteLine::addVoteLine(std::string const & voteText)
{
    std::string const timeNow = getHourMinuteNow();
    std::ostringstream oss;
    oss << timeNow << " " << voteText << '\n';
    text_->copy_label(oss.str().c_str());
}

bool VoteLine::processHostMessage(std::string const & msg)
{
    std::string voteText;

    voteText = checkSpringieVote(msg);
    if (!voteText.empty())
    {
        addVoteLine(voteText);
        return true;
    }

    voteText = checkSpadsVote(msg);
    if (!voteText.empty())
    {
        addVoteLine(voteText);
        return true;
    }

    return false;
}

/*
void VoteLine::processHostMessage(std::string const & msg)
{
    size_t pos;

    if ((pos = msg.find("? !vote 1 = yes, !vote 2 = no")) != std::string::npos)
    {
        // new vote
        question_ = msg.substr(0, pos+1).c_str();
        votesYes_ = votesNo_ = votesTotal_ = 0;
        makeVotesString();
        copy_label((question_ + votes_).c_str());
        activate();
    }
    else if (msg.find("option ") == 0) // optimization
    {
        // yes or no vote
        boost::regex re("option ([12]) has (\\d+) of (\\d+) votes");
        boost::smatch m;
        if (boost::regex_search(msg, m, re))
        {
            if (m[1] == "1")
            {
                votesYes_ = boost::lexical_cast<int>(m[2]);
            }
            else if (m[1] == "2")
            {
                votesNo_ = boost::lexical_cast<int>(m[2]);
            }
            else
            {
                LOG(WARNING)<< "expected [12]: '" << msg << "'";
                throw std::runtime_error("expected [12]");
            }
            votesTotal_ = boost::lexical_cast<int>(m[3]);
            makeVotesString();

            copy_label((question_ + votes_).c_str());
        }
        else
        {
            LOG(WARNING)<< "expected match for: '" << msg << "'";
        }
    }
    else if (msg.find("vote successful") == 0 ||
             msg.find("not enough votes") == 0 ||
             msg.find("poll cancelled") == 0)
    {
        // vote end
        copy_label((question_ + votes_ + msg).c_str());
        deactivate();
    }
    else if (msg.find("There is no poll going on") == 0)
    {
        // no vote in progress
        copy_label(msg.c_str());
        deactivate();
    }
}

void VoteLine::makeVotesString()
{
    std::ostringstream oss;
    oss << " [" << votesYes_ << "y " << votesNo_ << "n of " << votesTotal_ << "] ";
    votes_ = oss.str();
}
*/

int VoteLine::handle(int event)
{
    switch (event)
    {
    case FL_PUSH: // handle double click on text with web link
        if (Fl::event_clicks() && Fl::event_button() == FL_LEFT_MOUSE)
        {
            std::string const text = text_->label();
            auto const posStart = text.find("http://");
            if (posStart != std::string::npos)
            {
                auto const posEnd = text.find(' ', posStart);

                std::string const link(text, posStart, posEnd == std::string::npos ? posEnd : posEnd - posStart);
                LOG(DEBUG) << "link: '" << link << "'";

                char msg[512];
                int const res = fl_open_uri(link.c_str(), msg, sizeof(msg));
                if (res == 1)
                {
                    LOG(DEBUG)<< "fl_open_uri success: " << msg;
                }
                else // 0
                {
                    LOG(WARNING)<< "fl_open_uri failed: " << msg;
                }
                return 1;
            }
        }
        break;
    }

    return Fl_Group::handle(event);
}
