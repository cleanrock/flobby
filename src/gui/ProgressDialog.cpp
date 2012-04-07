#include "ProgressDialog.h"

#include <FL/Fl_Progress.H>
#include <FL/Fl.H>
#include <iostream> // TODO remove

ProgressDialog::ProgressDialog():
    Fl_Window(400, 100)
{
    progress_ = new Fl_Progress(50, 30, 300, 30);

    end();
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::progress(float percentage, std::string const & text)
{
    progress_->label(text.c_str());
    progress_->value(percentage);
    Fl::check();
    // ::usleep(10000); // TODO remove
}
