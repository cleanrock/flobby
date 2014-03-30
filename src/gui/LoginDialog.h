// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <boost/signals2/signal.hpp>
#include <string>

class Model;
class Fl_Input;
class Fl_Secret_Input;
class Fl_Box;
class Fl_Check_Button;

class LoginDialog: public Fl_Window
{
public:
    LoginDialog(Model & model);
    virtual ~LoginDialog();

    bool autoLogin() const;
    void attemptLogin();
    void show();

private:
    Model & model_;
    Fl_Preferences prefs_;

    Fl_Input * host_;
    Fl_Input * port_;
    Fl_Input * userName_;
    Fl_Secret_Input * password_;
    Fl_Check_Button * autoLogin_;
    Fl_Box * info_;

    std::string passwordHash_;
    bool loginInProgress_;

    static void callback(Fl_Widget*, void*);

    void loadLoginPrefs();
    void onLogin();

    // model signals
    void connected(bool connected);
    void loginResult(bool success, std::string const & info);

};
