#pragma once

#include "model/IController.h"
#include "IServerEvent.h"
#include "gui/UserInterface.h"

#include <boost/thread.hpp>
#include <deque>
#include <map>
#include <string>
#include <memory>

// forwards
//
class Model;
class ServerConn;

class Controller : public IController, public IServerEvent
{
public:
    Controller();
    virtual ~Controller();

    void model(Model & model) { model_ = &model; }
    void userInterface(UserInterface & ui) { ui_ = &ui; }

    // IController (called by model)
    void setIControllerEvent(IControllerEvent & iControllerEvent);
    void connect(std::string const & host, std::string const & service);
    void disconnect();
    void send(std::string const msg);
    unsigned int startProcess(std::string const & cmd, bool logToFile = false); // e.g. "/usr/bin/spring script.txt"

private:
    IControllerEvent * client_;
    Model * model_;
    UserInterface * ui_;
    bool connected_;
    std::unique_ptr<ServerConn> server_;

    typedef std::deque<bool> ConnectedQueue;
    ConnectedQueue connectedQueue_;

    typedef std::deque<std::string> RecvQueue;
    RecvQueue recvQueue_;

    boost::mutex mutexConnected_;
    boost::mutex mutexRecv_;
    boost::mutex mutexProcess_;

    // IServerEvent (called by server_ from its own thread)
    //
    void connected(bool connected);
    void message(std::string const & msg);

    // FLTK callbacks
    //
    static void connectedCallback(void * data);
    static void messageCallback(void * data);

    unsigned int processId_;
    void runProcess(std::string const & cmd, bool logToFile, unsigned int id);
    static void processDoneCallback(void * data);
    std::map<unsigned int, boost::thread*> procs_;
    std::vector<std::pair<unsigned int,int>> procsDone_; // procId, process (system) return value
};
