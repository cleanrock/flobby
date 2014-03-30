// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "TextDialog.h"

#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Return_Button.H>


TextDialog::TextDialog(char const * title, char const * textHeader):
    Fl_Window(400, 400, title)
{
    set_modal();

    text_ = new Fl_Multiline_Input(10, 30, 380, 300, textHeader);
    text_->align(FL_ALIGN_TOP_LEFT);

    auto * btn = new Fl_Return_Button(300, 350, 90, 30, "Save");
    btn->callback(TextDialog::callback, this);

    end();
}

TextDialog::~TextDialog()
{
}

void TextDialog::callback(Fl_Widget*, void *data)
{
    TextDialog * o = static_cast<TextDialog*>(data);
    o->onSave();
}

void TextDialog::onSave()
{
    textSaveSignal_(text_->value());
    hide();
}

void TextDialog::show(char const * text)
{
    if (text != 0)
    {
        text_->value(text);
    }
    Fl_Window::show();
}

