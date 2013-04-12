#include "ProgressDialog.h"

#include <FL/Fl_Progress.H>
#include <FL/Fl.H>

#include <cassert>

static ProgressDialog* instance_ = 0;

ProgressDialog::ProgressDialog():
    Fl_Window(400, 100)
{
    assert(instance_ == 0);
    instance_ = this;
    progress_ = new Fl_Progress(50, 30, 300, 30);

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

void ProgressDialog::progress(float percentage, std::string const & text)
{
    assert(instance_ != 0);

    instance_->progress_->copy_label(text.c_str());
    instance_->progress_->value(percentage);
    Fl::check();
}
