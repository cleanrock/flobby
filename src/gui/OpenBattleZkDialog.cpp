// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "OpenBattleZkDialog.h"
#include "Prefs.h"
#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Return_Button.H>

// prefs
char const * const PrefType = "Type";

OpenBattleZkDialog::OpenBattleZkDialog(Model& model)
    : Fl_Window(400, 400, "Open Battle")
    , model_(model)
    , prefs_(prefs(), "OpenBattleZk")
{
    set_modal();

    type_ = new Fl_Choice(10, 30, 380, 30, "Type");
    type_->align(FL_ALIGN_TOP_LEFT);
    int i;
    i = type_->add("Cooperative");  index2type_[i] = 5; // see enum AutohostMode in zk
    i = type_->add("Teams");        index2type_[i] = 6;
    i = type_->add("1v1");          index2type_[i] = 1;
    i = type_->add("FFA");          index2type_[i] = 4;
    i = type_->add("Custom");       index2type_[i] = 0;
    type_->callback(OpenBattleZkDialog::callbackType, this);
    prefs_.get(PrefType, i, 0);
    type_->value(i);

    title_ = new Fl_Input(10, 100, 380, 30, "Title");
    title_->align(FL_ALIGN_TOP_LEFT);

    password_ = new Fl_Input(10, 170, 380, 30, "Password");
    password_->align(FL_ALIGN_TOP_LEFT);


    Fl_Return_Button * btn = new Fl_Return_Button(300, 360, 90, 30, "Open");
    btn->callback(OpenBattleZkDialog::callbackOpen, this);

    end();
}

OpenBattleZkDialog::~OpenBattleZkDialog()
{
}

void OpenBattleZkDialog::open()
{
    setTitle();
    Fl_Window::show();
}

void OpenBattleZkDialog::setTitle()
{
    std::string const title = model_.me().name() + "'s " + type_->text(type_->value());
    title_->value(title.c_str());
}

void OpenBattleZkDialog::callbackType(Fl_Widget*, void *data)
{
    OpenBattleZkDialog* o = static_cast<OpenBattleZkDialog*>(data);
    o->setTitle();
}

void OpenBattleZkDialog::callbackOpen(Fl_Widget*, void *data)
{
    OpenBattleZkDialog * o = static_cast<OpenBattleZkDialog*>(data);

    o->model_.openBattle(o->index2type_[o->type_->value()], o->title_->value(), o->password_->value());
    o->prefs_.set(PrefType, o->type_->value());
    o->hide();
}
