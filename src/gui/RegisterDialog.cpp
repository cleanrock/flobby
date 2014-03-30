// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "RegisterDialog.h"
#include "Prefs.h"
#include "Sound.h"

#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <boost/bind.hpp>

RegisterDialog::RegisterDialog(Model & model):
    Fl_Window(400, 400, "Register"),
    model_(model),
    prefs_(prefs(), PrefLogin),
    registerInProgress_(false)
{
    set_modal();


    host_ = new Fl_Input(10, 30, 250, 30, "Host");
    host_->align(FL_ALIGN_TOP_LEFT);

    port_ = new Fl_Input(270, 30, 120, 30, "Port");
    port_->align(FL_ALIGN_TOP_LEFT);

    userName_ = new Fl_Input(10, 90, 380, 30, "User name");
    userName_->align(FL_ALIGN_TOP_LEFT);

    password_ = new Fl_Secret_Input(10, 150, 380, 30, "Password");
    password_->align(FL_ALIGN_TOP_LEFT);

    password2_ = new Fl_Secret_Input(10, 210, 380, 30, "Password repeat");
    password2_->align(FL_ALIGN_TOP_LEFT);

    email_ = new Fl_Input(10, 270, 380, 30, "E-mail (optional)");
    email_->align(FL_ALIGN_TOP_LEFT);
    email_->hide(); // TODO remove when email is supported

    info_ = new Fl_Box(10, 310, 380, 30);

    register_ = new Fl_Return_Button(280, 350, 110, 30, "Register");
    register_->callback(RegisterDialog::callback, this);

    end();

    // model signal handlers
    model.connectConnected( boost::bind(&RegisterDialog::connected, this, _1) );
    model.connectRegisterResult( boost::bind(&RegisterDialog::registerResult, this, _1, _2) );

}

RegisterDialog::~RegisterDialog()
{
}

void RegisterDialog::callback(Fl_Widget*, void *data)
{
    RegisterDialog * o = static_cast<RegisterDialog*>(data);
    o->onRegister();
}

void RegisterDialog::show()
{
    char str[128];
    prefs_.get(PrefLoginHost, str, "lobby.springrts.com", sizeof(str));
    host_->value(str);

    prefs_.get(PrefLoginPort, str, "8200", sizeof(str));
    port_->value(str);

    userName_->value(0);
    password_->value(0);
    password2_->value(0);
    email_->value(0);
    info_->label(0);

    focus(userName_);
    activate();
    Fl_Window::show();
}

void RegisterDialog::infoError(std::string const & text)
{
    info_->labelcolor(FL_RED);
    info_->copy_label(text.c_str());
    Sound::beep();
}

void RegisterDialog::onRegister()
{
    if (host_->size() == 0 || port_->size() == 0 || userName_->size() == 0 || password_->size() == 0)
    {
        infoError("missing input");
        return;
    }

    if (::strcmp(password_->value(), password2_->value()) != 0)
    {
        infoError("password mismatch");
        return;
    }

    registerInProgress_ = true;
    deactivate();
    model_.connect(host_->value(), port_->value());
}

void RegisterDialog::registerResult(bool success, std::string const & info)
{
    registerInProgress_ = false;
    model_.disconnect();

    if (!success)
    {
        infoError(info);
        activate();
        Fl_Window::show();
    }
    else
    {
        // save login prefs on successful register
        prefs_.set(PrefLoginHost, host_->value());
        prefs_.set(PrefLoginPort, port_->value());
        prefs_.set(PrefLoginUser, userName_->value());
        prefs_.set(PrefLoginPassword, passwordHash_.c_str());

        hide();
        fl_alert("Registration successful.\nYou can now login with your new account.");
    }
}

void RegisterDialog::connected(bool connected)
{
    if (registerInProgress_)
    {
        if (connected)
        {
            passwordHash_ = model_.calcPasswordHash(password_->value());

            // TODO email not yet supported
            model_.registerAccount(userName_->value(), passwordHash_, "" /*email_->value()*/);
        }
        else
        {
            infoError("no connection to server");
            registerInProgress_ = false;
            activate();
        }
    }
}
