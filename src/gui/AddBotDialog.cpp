// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "AddBotDialog.h"
#include "model/Model.h"

#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/lexical_cast.hpp>


AddBotDialog::AddBotDialog(Model & model):
    model_(model),
    Fl_Window(600, 400, "Add AI")
{
    set_modal();

    list_ = new Fl_Hold_Browser(0, 0, 200, 340);
    list_->callback(AddBotDialog::callbackList, this);

    buf_ = new Fl_Text_Buffer();
    info_ = new Fl_Text_Display(200, 0, 400, 340);
    info_->buffer(buf_);
    info_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    botName_ = new Fl_Input(10, 360, 200, 30, "AI name (must be unique)");
    botName_->align(FL_ALIGN_TOP_LEFT);

    add_ = new Fl_Return_Button(500, 360, 90, 30, "Add AI");
    add_->callback(AddBotDialog::callbackAdd, this);

    end();
}

AddBotDialog::~AddBotDialog()
{
}

void AddBotDialog::callbackList(Fl_Widget*, void *data)
{
    AddBotDialog * o = static_cast<AddBotDialog*>(data);
    o->onList();
}

void AddBotDialog::onList()
{
    buf_->remove(0, buf_->length());

    int const line = list_->value();
    if (line == 0)
    {
        add_->deactivate();
        return;
    }

    // add info
    std::string const aiName = list_->text(line);
    for (AI const & ai : ais_)
    {
        if (ai.name_ == aiName)
        {
            for (auto const & info : ai.info_)
            {
                buf_->append(info.first.c_str());
                buf_->append(": ");
                buf_->append(info.second.c_str());
                buf_->append("\n\n");
            }
            break;
        }
    }

    add_->activate();

    // add if double click
    if (Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks() != 0)
    {
        onAdd();
    }
}

void AddBotDialog::callbackAdd(Fl_Widget*, void *data)
{
    AddBotDialog * o = static_cast<AddBotDialog*>(data);
    o->onAdd();
}

void AddBotDialog::onAdd()
{
    int const line = list_->value();
    if (line > 0)
    {
        std::string const aiName = botName_->value();
        std::string const aiDll = list_->text(line);
        model_.addBot(Bot(aiName, aiDll));
        hide();
    }
}

void AddBotDialog::show(std::string const & game, std::string const & botName)
{
    list_->clear();
    buf_->remove(0, buf_->length());
    add_->deactivate();

    ais_ = model_.getModAIs(game);
    for (AI const & ai : ais_)
    {
        list_->add(ai.name_.c_str());
    }
    botName_->value(botName.c_str());
    Fl_Window::show();
}

