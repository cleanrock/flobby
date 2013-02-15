#include "FontSettingsDialog.h"
#include "Prefs.h"
#include "Sound.h"

#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


void FontSettingsDialog::setupFont()
{
    FL_NORMAL_SIZE = getFontSize();
}

int FontSettingsDialog::getFontSize()
{
    int fontSize;
    prefs.get(PrefFontSize, fontSize, 13);
    return fontSize;
}

FontSettingsDialog::FontSettingsDialog():
    Fl_Window(400, 400, "Font settings (change require restart)")
{
    set_modal();

    fontSize_ = new Fl_Int_Input(10, 30, 380, 30, "Font size, 10-16 is sane and safe");
    fontSize_->align(FL_ALIGN_TOP_LEFT);

    box_ = new Fl_Box(10, 170, 380, 30);
    box_->labelcolor(FL_RED);

    Fl_Return_Button * btn = new Fl_Return_Button(300, 350, 90, 30, "Apply");
    btn->callback(FontSettingsDialog::callback, this);

    end();
}

FontSettingsDialog::~FontSettingsDialog()
{
}

void FontSettingsDialog::callback(Fl_Widget*, void *data)
{
    FontSettingsDialog * o = static_cast<FontSettingsDialog*>(data);
    o->onApply();
}

void FontSettingsDialog::onApply()
{
    int fontSize = -1;
    try
    {
        fontSize = boost::lexical_cast<int>(fontSize_->value());
    }
    catch (boost::bad_lexical_cast & e)
    {
        // check below will display error
    }

    if (fontSize <= 0)
    {
        box_->label("Font size must be a positive number");
        Sound::beep();
        return;
    }

    prefs.set(PrefFontSize, fontSize);

    box_->label(0);
    hide();
}

void FontSettingsDialog::show()
{
    fontSize_->value(boost::lexical_cast<std::string>(getFontSize()).c_str());
    Fl_Window::show();
}

