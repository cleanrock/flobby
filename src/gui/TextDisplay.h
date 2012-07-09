#pragma once

#include <FL/Fl_Browser.H>
#include <string>

class Fl_Text_Buffer;

class TextDisplay: public Fl_Browser
{
public:
    TextDisplay(int x, int y, int w, int h, char const * label = 0);
    virtual ~TextDisplay();

    void append(std::string const & text); // prepends with time stamp and adds newline at end

private:
    bool scrollToBottom_;

    int handle(int event);
    void draw();

};
