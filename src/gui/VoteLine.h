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

    void processHostMessage(std::string const & text);

private:
    Model & model_;
    Fl_Box * text_;
    Fl_Button * yes_;
    Fl_Button * no_;

    static void onYes(Fl_Widget * w, void * data);
    static void onNo(Fl_Widget * w, void * data);

    // TODO keeping old for multiple host types
/*
    std::string question_;
    int votesYes_;
    int votesNo_;
    int votesTotal_;
    std::string votes_;

    void makeVotesString();
*/
};
