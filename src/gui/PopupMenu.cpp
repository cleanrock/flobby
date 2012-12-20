#include "PopupMenu.h"

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cstdint>

PopupMenu::PopupMenu():
    menu_(0,0,0,0)
{
    menu_.type(1); // non-zero means popup-menu
}

PopupMenu::~PopupMenu()
{
}

void PopupMenu::add(std::string const & text, int id)
{
    assert(id > 0);

    // escape label special chars
    std::string text2(text);
    if (!text2.empty() && text2[0] == '_')
    {
        text2.insert(0, "\\");
    }
    boost::replace_all(text2, "&", "\\&");
    boost::replace_all(text2, "/", "\\/");

    menu_.add(text2.c_str(), 0, 0, reinterpret_cast<void*>(static_cast<intptr_t>(id)) );
}

int PopupMenu::show()
{
    Fl_Menu_Item const * m = menu_.popup();

    if (m)
    {
        return reinterpret_cast<intptr_t>(m->user_data());
    }
    else
    {
        return 0;
    }
}

