#include "AddBotDialog.h"
#include "model/Model.h"

#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <boost/lexical_cast.hpp>


AddBotDialog::AddBotDialog(Model & model):
    model_(model),
    Fl_Window(400, 400, "Add AI")
{
    set_modal();

    list_ = new Fl_Hold_Browser(0, 0, 400, 340);
    list_->selection_color(FL_YELLOW);
    list_->callback(AddBotDialog::callbackList, this);

    botName_ = new Fl_Input(10, 360, 200, 30, "AI name (must be unique)");
    botName_->align(FL_ALIGN_TOP_LEFT);
    botName_->value("AI");

    Fl_Return_Button * btn = new Fl_Return_Button(300, 360, 90, 30, "Add AI");
    btn->callback(AddBotDialog::callbackButton, this);

    end();
}

AddBotDialog::~AddBotDialog()
{
}

void AddBotDialog::callbackList(Fl_Widget*, void *data)
{
    AddBotDialog * o = static_cast<AddBotDialog*>(data);
    // TODO reimplement Fl_Hold_Browser to only call callback on double click ???
}

void AddBotDialog::callbackButton(Fl_Widget*, void *data)
{
    AddBotDialog * o = static_cast<AddBotDialog*>(data);
    int const line = o->list_->value();
    if (line > 0)
    {
        std::string const aiName = o->botName_->value();
        std::string const aiDll = o->list_->text(line);
        o->model_.addBot(Bot(aiName, aiDll));
        o->hide();
    }
}

void AddBotDialog::show(std::string const & game)
{
    list_->clear();
    auto ais = model_.getModAIs(game);
    for (AI & ai : ais)
    {
        list_->add(ai.shortName_.c_str());
    }
    Fl_Window::show();
}

