#pragma once

#include <FL/Fl_Window.H>
#include <string>


class Fl_Progress;
class Fl_Button;

class ProgressDialog: public Fl_Window
{
public:
    ProgressDialog();
    virtual ~ProgressDialog();

    static void open(std::string const& title);
    static void progress(float percentage, std::string const & text);
    static void close();
    static bool isVisible();

private:
    Fl_Progress * progress_;
    Fl_Button * cancel_;

    static void callbackCancel(Fl_Widget* w, void *data);

};
