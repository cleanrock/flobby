#pragma once

#include <FL/Fl_Window.H>
#include <string>

class Model;
class LoginDialog;
class Fl_Multiline_Output;

class AgreementDialog: public Fl_Window
{
public:
    AgreementDialog(Model & model, LoginDialog & loginDialog);
    virtual ~AgreementDialog();

private:
    Model & model_;
    LoginDialog & loginDialog_;
    Fl_Multiline_Output * text_;

    static void callback(Fl_Widget*, void*);

    void agreement(std::string const & text);
    void onAccept();
};
