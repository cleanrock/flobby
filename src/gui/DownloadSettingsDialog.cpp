#include "DownloadSettingsDialog.h"
#include "Prefs.h"
#include "model/Model.h"

#include <FL/Fl_File_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
//#include <boost/filesystem.hpp>

// prefs
char const * const PrefPrDownloaderExternal = "PrDownloaderExternal";
char const * const PrefPrDownloaderCmd = "PrDownloaderCmd";

DownloadSettingsDialog::DownloadSettingsDialog(Model & model) :
        model_(model), Fl_Window(400, 300, "Downloader")
{
    set_modal();

    useExternalPrDownloader_ = new Fl_Check_Button(10, 30, 380, 30, "Use external pr-downloader");
    useExternalPrDownloader_->callback(DownloadSettingsDialog::callbackExternal, this);

    prDownloaderCmd_ = new Fl_File_Input(10, 90, 360, 40, "External pr-downloader");
    prDownloaderCmd_->align(FL_ALIGN_TOP_LEFT);

    prDownloaderCmdBrowse_ = new Fl_Button(370, 90, 20, 40, "...");
    prDownloaderCmdBrowse_->callback(DownloadSettingsDialog::callbackBrowsePrDownloader, this);

    save_ = new Fl_Return_Button(200, 250, 190, 30, "Save");
    save_->callback(DownloadSettingsDialog::callbackSave, this);

    end();

    init();

    model_.useExternalPrDownloader(0 != useExternalPrDownloader_->value());
    model_.setPrDownloaderCmd(prDownloaderCmd_->value());
}

void DownloadSettingsDialog::init()
{
    int useExternalPrd;
    prefs().get(PrefPrDownloaderExternal, useExternalPrd, 0);
    useExternalPrDownloader_->value(useExternalPrd);

    char* str;
    prefs().get(PrefPrDownloaderCmd, str, "pr-downloader");
    prDownloaderCmd_->value(str);
    ::free(str);

    onExternal();
}

DownloadSettingsDialog::~DownloadSettingsDialog()
{
}

void DownloadSettingsDialog::callbackSave(Fl_Widget*, void *data)
{
    DownloadSettingsDialog * o = static_cast<DownloadSettingsDialog*>(data);
    o->onSave();
}

void DownloadSettingsDialog::callbackExternal(Fl_Widget*, void *data)
{
    DownloadSettingsDialog * o = static_cast<DownloadSettingsDialog*>(data);
    o->onExternal();
}

void DownloadSettingsDialog::callbackBrowsePrDownloader(Fl_Widget*, void *data)
{
    DownloadSettingsDialog * o = static_cast<DownloadSettingsDialog*>(data);
    o->onBrowsePrDownloader();
}

void DownloadSettingsDialog::onExternal()
{
    int const useExternal = useExternalPrDownloader_->value();

    if (0 != useExternal)
    {
        prDownloaderCmd_->activate();
        prDownloaderCmdBrowse_->activate();
    }
    else
    {
        prDownloaderCmd_->deactivate();
        prDownloaderCmdBrowse_->deactivate();
    }
}

void DownloadSettingsDialog::onSave()
{
    prefs().set(PrefPrDownloaderExternal, useExternalPrDownloader_->value());
    prefs().set(PrefPrDownloaderCmd, prDownloaderCmd_->value());

    model_.useExternalPrDownloader(0 != useExternalPrDownloader_->value());
    model_.setPrDownloaderCmd(prDownloaderCmd_->value());

    // flush prefs to make debugging easier
    prefs().flush();
    hide();
}

void DownloadSettingsDialog::onBrowsePrDownloader()
{
    std::string fileName;
    if (openFileDialog("Select pr-downloader", prDownloaderCmd_->value(), fileName))
    {
        prDownloaderCmd_->value(fileName.c_str());
    }
}

void DownloadSettingsDialog::show()
{
    init();
    Fl_Window::show();
}

bool DownloadSettingsDialog::openFileDialog(char const * title, char const * fileName, std::string & result)
{
    Fl_Native_File_Chooser fc;
    fc.options(Fl_Native_File_Chooser::NO_OPTIONS);
    fc.title(title);
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);

    if (fileName != 0 && ::strlen(fileName) > 0)
    {
        fc.preset_file(fileName);
    }
    else
    {
        fc.directory("/");
    }

    if (fc.show() == 0)
    {
        result = fc.filename();
        return true;
    }
    return false;
}
