#include "VoteLine.h"

#include "model/Model.h"
#include "logging.h"
#include <FL/Fl_Button.H>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp> // <regex> in gcc 4.7 still "empty"
#include <sstream>

VoteLine::VoteLine(int x, int y, int w, int h, Model & model):
    Fl_Group(x, y, w, h),
    model_(model),
    votesYes_(0),
    votesNo_(0),
    votesTotal_(0)
{
    box(FL_FLAT_BOX);
    align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    int const btnWidth = 20;

    yes_ = new Fl_Button(x + w - 2*btnWidth, y, btnWidth, h, "Yes");
    no_ = new Fl_Button(x + w - 1*btnWidth, y, btnWidth, h, "No");


    yes_->callback(VoteLine::onYes, this);
    no_->callback(VoteLine::onNo, this);

    end();

    makeVotesString();
}

VoteLine::~VoteLine()
{
}

void VoteLine::onYes(Fl_Widget * w, void * data)
{
    VoteLine * o = static_cast<VoteLine*>(data);

    o->model_.sayBattle("!vote 1");
}

void VoteLine::onNo(Fl_Widget * w, void * data)
{
    VoteLine * o = static_cast<VoteLine*>(data);

    o->model_.sayBattle("!vote 2");
}

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
