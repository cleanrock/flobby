#include "SoundSettingsDialog.h"
#include "Sound.h"
#include "Prefs.h"

#include "log/Log.h"

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Return_Button.H>

// Prefs
static char const * const PrefSound = "Sound";
static char const * const PrefSoundCommand = "SoundCommand";

SoundSettingsDialog::SoundSettingsDialog():
    Fl_Window(400, 400, "Sound settings")
{
    set_modal();

    enable_ = new Fl_Check_Button(10, 30, 380, 30, "Enable sound");

    command_ = new Fl_Input(10, 80, 380, 30, "Sound command");
    command_->align(FL_ALIGN_TOP_LEFT);

    Fl_Button * test = new Fl_Button(350, 110, 40, 30, "Test");
    test->callback(SoundSettingsDialog::callbackTest, this);

    Fl_Return_Button * btn = new Fl_Return_Button(300, 360, 90, 30, "Apply");
    btn->callback(SoundSettingsDialog::callbackApply, this);

    end();

    loadPrefs();
    savePrefs();
}

SoundSettingsDialog::~SoundSettingsDialog()
{
}

void SoundSettingsDialog::show()
{
    loadPrefs();
    Fl_Window::show();
}

void SoundSettingsDialog::loadPrefs()
{
    int val;

    prefs.get(PrefSound, val, 1);
    enable_->value(val);

    char * text;
    prefs.get(PrefSoundCommand, text, "xkbbell -v 100");
    command_->value(text);
    ::free(text);
}

void SoundSettingsDialog::savePrefs()
{
    prefs.set(PrefSound, enable_->value());

    prefs.set(PrefSoundCommand, command_->value());

    Sound::enable_ = enable_->value() == 1 ? true : false;
    Sound::command_ = command_->value();
}

void SoundSettingsDialog::callbackTest(Fl_Widget*, void *data)
{
    SoundSettingsDialog * o = static_cast<SoundSettingsDialog*>(data);
    Sound::beep(o->command_->value());
}

void SoundSettingsDialog::callbackApply(Fl_Widget*, void *data)
{
    SoundSettingsDialog * o = static_cast<SoundSettingsDialog*>(data);
    o->savePrefs();
    o->hide();
}

