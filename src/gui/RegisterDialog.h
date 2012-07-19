#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <boost/signal.hpp>
#include <string>

class Model;
class Fl_Input;
class Fl_Secret_Input;
class Fl_Box;
class Fl_Return_Button;

class RegisterDialog: public Fl_Window
{
public:
    RegisterDialog(Model & model);
    virtual ~RegisterDialog();

    void show();

private:
    Model & model_;
    Fl_Preferences prefs_;

    Fl_Input * host_;
    Fl_Input * port_;
    Fl_Input * userName_;
    Fl_Secret_Input * password_;
    Fl_Secret_Input * password2_;
    Fl_Input * email_;
    Fl_Box * info_;
    Fl_Return_Button * register_;

    bool registerInProgress_;
    std::string passwordHash_;

    static void callback(Fl_Widget*, void*);
    void onRegister();

    void infoError(std::string const & text);

    // model signals
    void connected(bool connected);
    void registerResult(bool success, std::string const & info);

};
