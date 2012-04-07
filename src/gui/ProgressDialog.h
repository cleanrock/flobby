#pragma once

#include <FL/Fl_Window.H>
#include <string>


class Fl_Progress;

class ProgressDialog: public Fl_Window
{
public:
    ProgressDialog();
    virtual ~ProgressDialog();

    void progress(float percentage, std::string const & text);

private:
    Fl_Progress * progress_;
};
