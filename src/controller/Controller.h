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
    void send(std::string const& msg);
    uint64_t lastSendTime() const;
    uint64_t timeNow() const;
    unsigned int startThread(boost::function<int()> function);
    void runThread(boost::function<int()> function, unsigned int id);

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
    boost::mutex mutexThreads_;

    // IServerEvent (called by server_ from its own thread)
    //
    void connected(bool connected);
    void message(std::string const & msg);

    // FLTK callbacks
    //
    static void connectedCallback(void * data);
    static void messageCallback(void * data);

    unsigned int nextThreadId_;
    static void threadDoneCallback(void * data);

    struct ThreadInfo
    {
        boost::thread* thread_;
        int result_;
        ThreadInfo(): thread_(nullptr), result_(-1) {}
        ThreadInfo(boost::thread* thread): thread_(thread), result_(-1) {}
    };
    typedef std::map<unsigned int, ThreadInfo> Threads;
    Threads threads_;
};
