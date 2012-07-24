#include "TextDisplay.h"

#include "log/Log.h"

#include <sstream>
#include <ctime>

TextDisplay::TextDisplay(int x, int y, int w, int h, char const * label):
    Fl_Browser(x, y, w, h, label),
    scrollToBottom_(false)
{
    textsize(12);

    static int colWidths[] = { 65, 0 };
    column_widths(colWidths);

    // always show scrollbars until i can get "scroll to bottom" to work when horz scrollbar pops up
    has_scrollbar(BOTH_ALWAYS);
}

TextDisplay::~TextDisplay()
{
}

void TextDisplay::append(std::string const & text, bool interesting)
{
    char buf[16];

    // only show time when string non-empty
    if (!text.empty())
    {
        // time stamp
        std::time_t t = std::time(0);
        std::tm tm = *std::localtime(&t);
        // TODO replace with std::put_time when its available in gcc
        std::strftime(buf, 16, "%H:%M:%S", &tm);
    }
    else
    {
        strcpy(buf, " ");
    }
    std::ostringstream oss;
    oss << "@C" << FL_GRAY << "@." << buf << '\t'; // time in gray in first column
    if (!interesting)
    {
        oss << "@C" << FL_DARK2 << "@.";
    }
    oss << text;

    // make sure we scroll to bottom when last line is visible
    if (scrollToBottom_ == false && size() > 0 && displayed(size()))
    {
        scrollToBottom_ = true;
    }

    add(oss.str().c_str());

    // limit number of lines
    while (size() > 200)
    {
        remove(1);
    }
}

int TextDisplay::handle(int event)
{
    // make mouse wheel scroll half a page
    if (event == FL_MOUSEWHEEL)
    {
        int const rowsPerPage = h()/incr_height();
        Fl::e_dy *= std::max(1, rowsPerPage/2);
    }

    switch (event)
    {
    case FL_PUSH: // handle double click on lines with web link
        if (Fl::event_clicks() && Fl::event_button() == FL_LEFT_MOUSE)
        {
            void * item = find_item(Fl::event_y());
            if (item)
            {
                std::string const text = item_text(item);
                size_t pos = text.find("http://");
                if (pos != std::string::npos)
                {
                    size_t end = text.find_first_of(" \t\r\n\v\f", pos);
                    std::string const link = text.substr(pos, end-pos);
                    LOG(DEBUG) << "link: '" << link << "'";

                    std::string const cmd = "xdg-open " + link;
                    std::system(cmd.c_str());
                }
            }
            return 1;
        }
        break;
    }
    return Fl_Browser::handle(event);
}

void TextDisplay::draw()
{
    if (scrollToBottom_)
    {
        bottomline(size());
        scrollToBottom_ = false;
    }
    Fl_Browser::draw();
}
