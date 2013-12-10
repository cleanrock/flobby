#include "FlobbyDirs.h"
#include "controller/Controller.h"
#include "model/Model.h"
#include "gui/UserInterface.h"
#include <pr-downloader.h>

// TODO #include <boost/program_options.hpp>
// #include <iostream>
//#include <cstdlib>
// TODO std::vector<std::string> parseOptions(int argc, char * argv[]);

int main(int argc, char * argv[])
{
    /* TODO using prefs instead of commandline for now
        std::vector<std::string> rest = parseOptions(argc, argv);

        // build remaining argc and argv (for fltk)
        int argcRest = 1+rest.size();
        char * argvRest[argcRest];
        argvRest[0] = argv[0];
        for (int i=0; i<rest.size(); ++i)
        {
            argvRest[i+1] = (char*)rest[i].c_str();
        }
    */

    initDirs();

    // init pr-downloader
    DownloadInit();
    DownloadDisableLogging(true);

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
        // TODO ui.run(argcRest, argvRest);
    }

    // shutdown pr-downloader
    DownloadShutdown();

    return 0;
}

/* TODO
std::vector<std::string> parseOptions(int argc, char * argv[])
{
    namespace po = boost::program_options;

    std::string logFile;
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "produce help message")
        ("logfile,f", po::value<std::string>(&logFile)->default_value("/tmp/flobby.log"), "log file")
        ("debug,d", "enable debug log messages")
    ;

    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();

    po::variables_map vm;
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        ::exit(0);
    }

    if (vm.count("debug"))
    {
        Log::minSeverity(DEBUG);
    }
    Log::logFile(logFile);

    return po::collect_unrecognized(parsed.options, po::include_positional);
}
*/
