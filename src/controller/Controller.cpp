#include "Controller.h"
#include "ServerConn.h"
#include "model/Model.h"
#include "IServerEvent.h"
#include "log/Log.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <thread>
#include <cstdlib>

Controller::Controller():
    client_(0),
    model_(0),
    ui_(0),
    connected_(false),
    processId_(0)
{
}

Controller::~Controller()
{
}

void Controller::setIControllerEvent(IControllerEvent & iControllerEvent)
{
    client_ = &iControllerEvent;
}

void Controller::connect(const std::string & host, const std::string & service)
{
    server_.reset(new ServerConn(host, service, *this));
}

void Controller::send(const std::string msg)
{
    if (!server_)
    {
        throw std::runtime_error("not connected");
    }
    server_->send(msg);
}

unsigned int Controller::startProcess(std::string const & cmd, bool logToFile)
{
    ++processId_;
    processId_ = std::max(processId_, 1U); // make sure it doesn't wrap to zero (not very likely though)

    std::thread * t = new std::thread(boost::bind(&Controller::runProcess, this, cmd, logToFile, processId_));
    procs_[processId_] = t;

    return processId_;
}

void Controller::runProcess(std::string const & cmd, bool logToFile, unsigned int id)
{
    LOG(DEBUG) << "runProcess: '" << cmd << "'";

    int ret;
    if (logToFile)
    {
        // create log filename
        std::string const first = cmd.substr(0, cmd.find(' '));
        boost::filesystem::path const path(first);
        std::string const log = "flobby_process_" + path.stem().string() + ".log";
        LOG(DEBUG) << "runProcess logFile: '" << log << "'";

        // redirect stdout and stderr to log file
        std::string cmdLine = cmd + " >> " + log + " 2>&1";

        LOG(DEBUG) << "runProcess system(): '" << cmdLine << "'";
        ret = std::system(cmdLine.c_str());

        // TODO remove this when i know pr-downloader returns sane value
        std::ostringstream oss;
        oss << "echo \"system returned " << ret << "\" >> " << log;
        std::system(oss.str().c_str());
    }
    else
    {
        std::string cmdLine = cmd + " >> /dev/null" + " 2>&1";
        LOG(DEBUG) << "runProcess system(): '" << cmdLine << "'";
        ret = std::system(cmdLine.c_str());
    }

//    FILE * f = ::popen(cmd.c_str(), "re"); // e = close-on-exec
//    if (f != NULL)
//    {
//        int const bufSize = 256;
//        char buf[bufSize];
//        while (::fgets(buf, bufSize, f) != NULL)
//        {
//            // TODO write to file ???
//        }
//        ::pclose(f);
//    }

    {
        boost::lock_guard<boost::mutex> lock(mutexProcess_);
        procsDone_.push_back(std::make_pair(id, ret));
    }
    ui_->addCallbackEvent(&processDoneCallback, this);
}

void Controller::processDoneCallback(void * data)
{
    Controller* c = static_cast<Controller*>(data);

    boost::lock_guard<boost::mutex> lock(c->mutexProcess_);
    for (std::pair<unsigned int, int> const & idRetPair : c->procsDone_)
    {
        c->client_->processDone(idRetPair);
        std::map<unsigned int, std::thread*>::iterator it = c->procs_.find(idRetPair.first);
        if (it != c->procs_.end())
        {
            it->second->join();
            delete it->second;
            // TODO delete entry in procs_
        }
        else
        {
            LOG(WARNING) << "thread not found:" << idRetPair.first;
        }
    }
    c->procsDone_.clear();
}

void Controller::connected(bool connected)
{
    {
        boost::lock_guard<boost::mutex> lock(mutexConnected_);
        connectedQueue_.push_back(connected);
        LOG_IF(DEBUG, connectedQueue_.size() > 1) << "connectedQueue_.size():" << connectedQueue_.size();
    }
    ui_->addCallbackEvent(&connectedCallback, this);
}

void Controller::disconnect()
{
    server_.reset();
}

void Controller::message(std::string const & msg)
{
    {
        boost::lock_guard<boost::mutex> lock(mutexRecv_);
        recvQueue_.push_back(msg);
        LOG_IF(DEBUG, recvQueue_.size() > 1) << "recvQueue_.size():" << recvQueue_.size();
    }

    ui_->addCallbackEvent(&messageCallback, this);
}

void Controller::connectedCallback(void * data)
{
    Controller* c = static_cast<Controller*>(data);

    boost::lock_guard<boost::mutex> lock(c->mutexConnected_);

    while (!c->connectedQueue_.empty())
    {
        c->client_->connected(c->connectedQueue_.front());
        c->connectedQueue_.pop_front();
    }
}



void Controller::messageCallback(void *data)
{
    Controller* c = static_cast<Controller*>(data);

    boost::lock_guard<boost::mutex> lock(c->mutexRecv_);

    while (!c->recvQueue_.empty())
    {
        c->client_->message(c->recvQueue_.front());
        c->recvQueue_.pop_front();
    }
}
