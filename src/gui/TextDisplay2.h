#pragma once

#include <FL/Fl_Text_Display.H>
#include <string>

class Fl_Text_Buffer;

class TextDisplay2: public Fl_Text_Display
{
public:
    TextDisplay2(int x, int y, int w, int h, char const * label = 0);
    virtual ~TextDisplay2();

    void append(std::string const & text, int interest = 0); // prepends with time stamp and adds newline at end

    enum {
        STYLE_TIME = 0,
        STYLE_LOW,
        STYLE_NORMAL,
        STYLE_HIGH,
        STYLE_MYTEXT,
        STYLE_COUNT
    };
    static Fl_Text_Display::Style_Table_Entry textStyles_[STYLE_COUNT];
    static void initTextStyles(); // call after setting font size

private:
    Fl_Text_Buffer * text_;
    Fl_Text_Buffer * style_;

    int handle(int event);

};
