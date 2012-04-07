#include "VoteLine.h"

#include "model/Model.h"

#include <FL/Fl_Button.H>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>

VoteLine::VoteLine(int x, int y, int w, int h, Model & model):
    Fl_Group(x, y, w, h),
    model_(model)
{
    box(FL_FLAT_BOX);
    align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    int const btnWidth = 20;

    yes_ = new Fl_Button(x + w - 2*btnWidth, y, btnWidth, h, "Yes");
    no_ = new Fl_Button(x + w - 1*btnWidth, y, btnWidth, h, "No");


    yes_->callback(VoteLine::onYes, this);
    no_->callback(VoteLine::onNo, this);

    end();

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
        copy_label(msg.substr(0, pos+1).c_str());
        activate();
    }
    else if (msg.find("vote successful") == 0 ||
             msg.find("not enough votes") == 0 ||
             msg.find("poll cancelled") == 0)
    {
        // vote end
        deactivate();
    }
}
