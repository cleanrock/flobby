#pragma once

#include <FL/Fl_Window.H>
#include <string>

class Model;
class Fl_Input;
class Fl_File_Input;
class Fl_Button;
class Fl_Check_Button;
class Fl_Return_Button;

class DownloadSettingsDialog: public Fl_Window
{
public:
    DownloadSettingsDialog(Model & model);
    virtual ~DownloadSettingsDialog();

    void show();
    void init();


private:
    Model & model_;

    Fl_Check_Button * useExternalPrDownloader_;
    Fl_File_Input * prDownloaderCmd_;
    Fl_Button * prDownloaderCmdBrowse_;
    Fl_Return_Button * save_;

    static void callbackExternal(Fl_Widget*, void*);
    static void callbackBrowsePrDownloader(Fl_Widget*, void*);
    static void callbackSave(Fl_Widget*, void*);

    void onExternal();
    void onSave();
    void onBrowsePrDownloader();
    bool openFileDialog(char const * title, char const * fileName, std::string & result); // returns false on cancel
};
