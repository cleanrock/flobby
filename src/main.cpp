// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "FlobbyDirs.h"
#include "FlobbyConfig.h"
#include "log/Log.h"
#include "controller/Controller.h"
#include "model/Model.h"
#include "gui/UserInterface.h"
#include <FL/Fl.H>
// TODO #include <pr-downloader.h>

static std::string dir_;

static
void printUsage(char const* argv0, std::string const& errorMsg = "")
{
    char const* usage =
        "%s" // errorMsg
        "usage: %s [options]\n"
        " -d | --dir <dir> : use <dir> for flobby config and cache instead of XDG\n"
        " -v | --version   : print flobby version\n"
        " -h | --help      : print help message\n"
        " plus standard fltk options:\n"
        "%s\n";

    Fl::fatal(usage, errorMsg.c_str(), argv0, Fl::help);
}

static
int parseArgs(int argc, char** argv, int& i)
{
    if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
    {
        printUsage(argv[0]);
    }

    if (strcmp("-d", argv[i]) == 0 || strcmp("--dir", argv[i]) == 0)
    {
        if (i < argc-1 && argv[i+1] != 0)
        {
            dir_ = argv[i+1];
            i += 2;
            return 2;
        }
    }

    if (strcmp("-v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0)
    {
        Fl::fatal("flobby version %s\n", FLOBBY_VERSION);
    }

    return 0;
}

int main(int argc, char * argv[])
{
    int i = 1;
    if (Fl::args(argc, argv, i, parseArgs) < argc)
    {
        std::string errorMsg = "error: unknown option: ";
        errorMsg += argv[i];
        errorMsg += "\n";
        printUsage(argv[0], errorMsg);
    }

    LOG(INFO)<< "starting flobby "<< FLOBBY_VERSION;
    initDirs(dir_);

    /* TODO disable static pr-d for now (91.0 unitsync cause crash when trying engine download)
    // init pr-downloader
    DownloadInit();
    DownloadDisableLogging(true);
    */

    // extra scope to be able to check destruction
    {
        // setup
        UserInterface::setupEarlySettings();

        Controller controller;
        Model model(controller);
        UserInterface ui(model);
        controller.model(model);
        controller.userInterface(ui);

        // start
        ui.run(argc, argv);
    }

    // shutdown pr-downloader
    // TODO DownloadShutdown();

    return 0;
}
