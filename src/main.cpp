#include "logging.h"
#include "model/Model.h"
#include "controller/Controller.h"
#include "gui/UserInterface.h"

int main(int argc, char * argv[])
{
    google::InitGoogleLogging(argv[0]);
    // google::InstallFailureSignalHandler();
    google::SetStderrLogging(google::WARNING);

    {
        // setup
        Controller controller;
        Model model(controller);
        UserInterface ui(model);
        controller.model(model);
        controller.userInterface(ui);

        // start
        ui.run(argc, argv);
    }

    return 0;
}
