#pragma once

#include <FL/Fl_Group.H>
#include <string>

class Model;
class Fl_Label;
class Fl_Button;
class Fl_Widget;

class VoteLine: public Fl_Group
{
public:
    VoteLine(int x, int y, int w, int h, Model & model);
    virtual ~VoteLine();

    void processHostMessage(std::string const & text);

private:
    Model & model_;
    Fl_Button * yes_;
    Fl_Button * no_;

    static void onYes(Fl_Widget * w, void * data);
    static void onNo(Fl_Widget * w, void * data);
};
