#include "LoginDialog.h"
#include "Prefs.h"
#include "md5/md5.h"
#include "md5/base64.h"

#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <boost/bind.hpp>

LoginDialog::LoginDialog(Model & model):
    Fl_Window(400, 400, "Login"),
    model_(model), // TODO remove ?
    prefs_(prefs, label())
{
    set_modal();

    char str[65];

    host_ = new Fl_Input(10, 30, 250, 30, "Host");
    host_->align(FL_ALIGN_TOP_LEFT);
    prefs_.get(host_->label(), str, "springrts.com", 64);
    host_->value(str);

    port_ = new Fl_Input(270, 30, 120, 30, "Port");
    port_->align(FL_ALIGN_TOP_LEFT);
    prefs_.get(port_->label(), str, "8200", 64);
    port_->value(str);

    userName_ = new Fl_Input(10, 100, 380, 30, "User name");
    userName_->align(FL_ALIGN_TOP_LEFT);
    prefs_.get(userName_->label(), str, "", 64);
    userName_->value(str);

    password_ = new Fl_Secret_Input(10, 170, 380, 30, "Password");
    password_->align(FL_ALIGN_TOP_LEFT);
    prefs_.get(password_->label(), str, "", 64);
    passwordHash_ = str;
    password_->value(str);

    info_ = new Fl_Box(10, 200, 380, 100);
    info_->labelcolor(FL_RED);

    Fl_Return_Button * btn = new Fl_Return_Button(300, 350, 90, 30, "Login");
    btn->callback(LoginDialog::callback, this);

    end();

    // model signal handlers
    model.connectLoginResult( boost::bind(&LoginDialog::loginResult, this, _1, _2) );

}

LoginDialog::~LoginDialog()
{
    prefs_.set(host_->label(), host_->value());
    prefs_.set(port_->label(), port_->value());
    prefs_.set(userName_->label(), userName_->value());
    prefs_.set(password_->label(), passwordHash_.c_str());
}

void LoginDialog::callback(Fl_Widget*, void *data)
{
    LoginDialog * o = static_cast<LoginDialog*>(data);
    o->onLogin();
}

void LoginDialog::onLogin()
{
    if (password_->changed())
    {
        md5_state_t md5;
        md5_init(&md5);
        std::string const pw(password_->value());
        md5_append(&md5, (md5_byte_t const *)pw.data(), pw.size());

        md5_byte_t result[16];
        md5_finish(&md5, result);

        passwordHash_ = base64_encode(result, 16);
        password_->value(passwordHash_.c_str());
    }

    model_.login(
        host_->value(),
        port_->value(),
        userName_->value(),
        passwordHash_ );

    deactivate();
}

void LoginDialog::loginResult(bool success, std::string const & info)
{
    activate();
    if (!success)
    {
        show();
        info_->copy_label(info.c_str());
        fl_beep();
    }
    else
    {
        info_->label(0);
        hide();
    }
}
