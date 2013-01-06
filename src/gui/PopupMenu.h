#pragma once

#include <FL/Fl_Menu_Button.H>
#include <string>

class PopupMenu
{
public:
    PopupMenu();
    virtual ~PopupMenu();

    void add(std::string const & text, int id, bool escapeFltkChars = true); // id must be > 0
    int size() { return menu_.size(); }
    int show(); // returns selected item or 0 if none

private:
    Fl_Menu_Button menu_;

};
