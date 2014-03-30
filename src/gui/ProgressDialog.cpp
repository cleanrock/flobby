// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "ProgressDialog.h"

#include <FL/Fl_Progress.H>
#include <FL/Fl_Button.H>
#include <FL/Fl.H>

#include <cassert>

static ProgressDialog* instance_ = 0;

ProgressDialog::ProgressDialog():
    Fl_Window(400, 130)
{
    assert(instance_ == 0);
    instance_ = this;
    progress_ = new Fl_Progress(50, 30, 300, 30);
    cancel_ = new Fl_Button(150, 80, 100, 30, "Cancel");
    cancel_->callback(ProgressDialog::callbackCancel, this);

    set_modal();
    end();
}

ProgressDialog::~ProgressDialog()
{
    instance_ = 0;
}

void ProgressDialog::open(std::string const& title)
{
    assert(instance_ != 0);

    instance_->copy_label(title.c_str());
    instance_->show();
    progress(0, "");
    Fl::check();
}

void ProgressDialog::close()
{
    assert(instance_ != 0);

    instance_->hide();
}

bool ProgressDialog::isVisible()
{
    assert(instance_ != 0);

    return instance_->visible();
}

void ProgressDialog::progress(float percentage, std::string const & text)
{
    assert(instance_ != 0);

    instance_->progress_->copy_label(text.c_str());
    instance_->progress_->value(percentage);
    Fl::check();
}

void ProgressDialog::callbackCancel(Fl_Widget* w, void *data)
{
    // not needed atm
    // ProgressDialog* o = static_cast<ProgressDialog*>(data);

    ProgressDialog::close();
}
