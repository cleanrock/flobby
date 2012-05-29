#include "logging.h"
#include "TextDisplay.h"

#include <sstream>
#include <ctime>

TextDisplay::TextDisplay(int x, int y, int w, int h, char const * label):
    Fl_Browser(x, y, w, h, label)
{
    // TODO box(FL_THIN_DOWN_FRAME);
    textsize(12);
    // TODO wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    static int colWidths[] = { 65, 0 };
    column_widths(colWidths);
}

TextDisplay::~TextDisplay()
{
}

void TextDisplay::append(std::string const & text)
{
    char buf[16];

    // only show time when string non-empty
    if (!text.empty())
    {
        // time stamp
        std::time_t t = std::time(0);
        std::tm tm = *std::localtime(&t);
        // TODO replace with std::put_time when its available in gcc
        strftime(buf, 16, "%H:%M:%S", &tm);
    }
    else
    {
        strcpy(buf, " ");
    }
    std::ostringstream oss;
    oss << "@C" << FL_GRAY << "@." << buf << '\t' // time in gray in first column
        << text;

    // hack to make sure we scroll when multiple appends are called
    float const scrollV = scrollbar.value();
    bool const scroll = (   (scrollV == 0 && scrollbar.maximum() == 1)
                         || (scrollV > 0 && scrollV == scrollbar.maximum()) );

    add(oss.str().c_str());

    // limit number of lines
    while (size() > 200)
    {
        remove(1);
    }

    if (scroll)
    {
        make_visible(size());
    }
}

int TextDisplay::handle(int event)
{
    // make mousewheel scroll half a page
    if (event == FL_MOUSEWHEEL)
    {
        int const rowsPerPage = h()/incr_height();
        Fl::e_dy *= std::max(1, rowsPerPage/2);
    }

    switch (event)
    {
    case FL_PUSH:
        if (Fl::event_clicks() && Fl::event_button() == FL_LEFT_MOUSE)
        {
            void * item = find_item(Fl::event_y());
            if (item)
            {
                // TODO replace with std::regex when it works

                std::string const text = item_text(item);
                size_t pos = text.find("http://");
                if (pos != std::string::npos)
                {
                    size_t end = text.find_first_of(" \t\r\n\v\f", pos);
                    std::string const link = text.substr(pos, end-pos);
                    DLOG(INFO) << "link: '" << link << "'";

                    std::string const cmd = "xdg-open " + link;
                    ::system(cmd.c_str());
                }
            }
            return 1;
        }
        break;
    }
    return Fl_Browser::handle(event);
}

