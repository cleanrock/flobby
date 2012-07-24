#pragma once

#include <FL/Fl_Text_Display.H>
#include <string>

class Fl_Text_Buffer;

class TextDisplay2: public Fl_Text_Display
{
public:
    TextDisplay2(int x, int y, int w, int h, char const * label = 0);
    virtual ~TextDisplay2();

    void append(std::string const & text, bool interesting = true); // prepends with time stamp and adds newline at end

private:
    Fl_Text_Buffer * text_;
    Fl_Text_Buffer * style_;

    int handle(int event);

};
