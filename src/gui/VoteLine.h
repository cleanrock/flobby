// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Group.H>
#include <string>

class Model;
class Fl_Box;
class Fl_Button;

class VoteLine: public Fl_Group
{
public:
    VoteLine(int x, int y, int w, int h, Model & model);
    virtual ~VoteLine();

    bool processHostMessage(std::string const & text);

private:
    Model & model_;
    Fl_Box * text_;
    Fl_Button * yes_;
    Fl_Button * no_;

    static void onYes(Fl_Widget * w, void * data);
    static void onNo(Fl_Widget * w, void * data);

    std::string checkSpringieVote(std::string const & msg);
    std::string checkSpadsVote(std::string const & msg);
    void addVoteLine(std::string const & voteText);
};
