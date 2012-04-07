#include "PopupMenu.h"
#include <cassert>

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
    menu_.add(text.c_str(), 0, 0, (void*)id);
}

int PopupMenu::show()
{
    Fl_Menu_Item const * m = menu_.popup();

    if (m)
    {
        return reinterpret_cast<int>(m->user_data());
    }
    else
    {
        return 0;
    }
}

