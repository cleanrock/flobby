#include "AgreementDialog.h"
#include "LoginDialog.h"
#include "model/Model.h"

#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Return_Button.H>
#include <boost/bind.hpp>

AgreementDialog::AgreementDialog(Model & model, LoginDialog & loginDialog):
    model_(model),
    loginDialog_(loginDialog),
    Fl_Window(600, 600, "Agreement")
{
    set_modal();

    text_ = new Fl_Multiline_Output(10, 30, 580, 500);
    text_->wrap(1);

    auto * btn = new Fl_Return_Button(500, 550, 90, 30, "Accept");
    btn->callback(AgreementDialog::callback, this);

    end();

    // model signals
    model_.connectAgreement( boost::bind(&AgreementDialog::agreement, this, _1) );
}

AgreementDialog::~AgreementDialog()
{
}

void AgreementDialog::callback(Fl_Widget*, void *data)
{
    AgreementDialog * o = static_cast<AgreementDialog*>(data);
    o->onAccept();
}

void AgreementDialog::agreement(std::string const & text)
{
    text_->value(text.c_str());
    show();
}

void AgreementDialog::onAccept()
{
    model_.confirmAgreement();
    loginDialog_.attemptLogin();
    hide();
}

