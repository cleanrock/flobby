#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <boost/signal.hpp>
#include <string>

class Model;
class Fl_Input;
class Fl_Secret_Input;
class Fl_Box;

class LoginDialog: public Fl_Window
{
public:
    LoginDialog(Model & model);
    virtual ~LoginDialog();

private:
    Model & model_;
    Fl_Preferences prefs_;

    Fl_Input * host_;
    Fl_Input * port_;
    Fl_Input * userName_;
    Fl_Secret_Input * password_;
    Fl_Box * info_;

    std::string passwordHash_;

    static void callback(Fl_Widget*, void*);
    void onLogin();
    void loginResult(bool success, std::string const & info);

};
