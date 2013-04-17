#include "LoginDialog.h"
#include "Prefs.h"
#include "Sound.h"

#include "md5/md5.h"
#include "md5/base64.h"

#include "model/Model.h"

#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <boost/bind.hpp>

// Prefs
static char const * const PrefAutoLogin = "AutoLogin";

LoginDialog::LoginDialog(Model & model):
    Fl_Window(400, 400, "Login"),
    model_(model),
    prefs_(prefs(), PrefLogin),
    loginInProgress_(false)
{
    set_modal();

    host_ = new Fl_Input(10, 30, 250, 30, "Host");
    host_->align(FL_ALIGN_TOP_LEFT);

    port_ = new Fl_Input(270, 30, 120, 30, "Port");
    port_->align(FL_ALIGN_TOP_LEFT);

    userName_ = new Fl_Input(10, 100, 380, 30, "User name");
    userName_->align(FL_ALIGN_TOP_LEFT);

    password_ = new Fl_Secret_Input(10, 170, 380, 30, "Password");
    password_->align(FL_ALIGN_TOP_LEFT);

    autoLogin_ = new Fl_Check_Button(10, 220, 380, 30, "Login automatically");

    info_ = new Fl_Box(10, 250, 380, 90);
    info_->labelcolor(FL_RED);

    Fl_Return_Button * btn = new Fl_Return_Button(300, 350, 90, 30, "Login");
    btn->callback(LoginDialog::callback, this);

    end();

    loadLoginPrefs();

    // model signal handlers
    model.connectConnected( boost::bind(&LoginDialog::connected, this, _1) );
    model.connectLoginResult( boost::bind(&LoginDialog::loginResult, this, _1, _2) );

}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::loadLoginPrefs()
{
    char str[128];

    prefs_.get(PrefLoginHost, str, "lobby.springrts.com", sizeof(str));
    host_->value(str);

    prefs_.get(PrefLoginPort, str, "8200", sizeof(str));
    port_->value(str);

    prefs_.get(PrefLoginUser, str, "", sizeof(str));
    userName_->value(str);

    prefs_.get(PrefLoginPassword, str, "", sizeof(str));
    passwordHash_ = str;
    password_->value(str);

    int autoLogin;
    prefs_.get(PrefAutoLogin, autoLogin, 0);
    autoLogin_->value(autoLogin);
}

void LoginDialog::show()
{
    loadLoginPrefs();
    Fl_Window::show();
}

void LoginDialog::callback(Fl_Widget*, void *data)
{
    LoginDialog * o = static_cast<LoginDialog*>(data);
    o->onLogin();
}

void LoginDialog::attemptLogin()
{
    loginInProgress_ = true;
    model_.connect(host_->value(), port_->value());
    deactivate();
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

    attemptLogin();
}

void LoginDialog::loginResult(bool success, std::string const & info)
{
    loginInProgress_ = false;
    activate();
    if (!success)
    {
        show();
        info_->copy_label(info.c_str());
        Sound::beep();
        model_.disconnect();
    }
    else
    {
        // save login prefs on successful login
        prefs_.set(PrefLoginHost, host_->value());
        prefs_.set(PrefLoginPort, port_->value());
        prefs_.set(PrefLoginUser, userName_->value());
        prefs_.set(PrefLoginPassword, passwordHash_.c_str());
        prefs_.set(PrefAutoLogin, static_cast<int>(autoLogin_->value()) );

        info_->label(0);
        hide();
    }
}

void LoginDialog::connected(bool connected)
{
    if (loginInProgress_)
    {
        loginInProgress_ = false;
        if (!connected)
        {
            activate();
            show();
            info_->copy_label("no connection");
        }
        else
        {
            model_.login(userName_->value(), passwordHash_);
        }
    }
}

bool LoginDialog::autoLogin() const
{
    return autoLogin_->value();
}
