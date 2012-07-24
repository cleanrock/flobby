// this class is not used
#pragma once

#include <FL/Fl_Browser.H>
#include <string>

class Fl_Text_Buffer;

class TextDisplay: public Fl_Browser
{
public:
    TextDisplay(int x, int y, int w, int h, char const * label = 0);
    virtual ~TextDisplay();

    void append(std::string const & text, bool interesting = true); // prepends with time stamp

private:
    bool scrollToBottom_;

    int handle(int event);
    void draw();

};
