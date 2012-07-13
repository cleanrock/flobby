#pragma once

#include <FL/Fl_Window.H>
#include <boost/signal.hpp>
#include <string>

class Fl_Multiline_Input;

class TextDialog: public Fl_Window
{
public:
    TextDialog(char const * title, char const * textHeader);
    virtual ~TextDialog();

    void show(char const * text = 0);

    // signals
    //
    typedef boost::signal<void (std::string const & text)> TextSaveSignal;
    boost::signals::connection connectTextSave(TextSaveSignal::slot_type subscriber)
    { return textSaveSignal_.connect(subscriber); }

private:
    Fl_Multiline_Input * text_;
    TextSaveSignal textSaveSignal_;

    static void callback(Fl_Widget*, void*);
    void onSave();
};
