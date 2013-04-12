#pragma once

#include <FL/Fl_Window.H>
#include <string>


class Fl_Progress;

class ProgressDialog: public Fl_Window
{
public:
    ProgressDialog();
    virtual ~ProgressDialog();

    static void open(std::string const& title);
    static void progress(float percentage, std::string const & text);
    static void close();

private:
    Fl_Progress * progress_;
};
