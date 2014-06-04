// This file is part of flobby (GPL v2 or later), see the LICENSE file

#include "Controller.h"
#include "ServerConn.h"
#include "model/Model.h"
#include "IServerEvent.h"
#include "log/Log.h"
#include "FlobbyDirs.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
#include <cstdlib>
#include <cassert>

using namespace boost::chrono;

static time_point<steady_clock> timeStart_ = steady_clock::now();
static time_point<steady_clock> timeLastSend_ = timeStart_;
static Controller* controller_ = nullptr;

Controller::Controller():
    client_(0),
    model_(0),
    ui_(0),
    connected_(false),
    nextThreadId_(1)
{
    // ugly singleton
    assert(controller_ == 0);
    controller_ = this;
}

Controller::~Controller()
{
}

void Controller::setIControllerEvent(IControllerEvent & iControllerEvent)
{
    client_ = &iControllerEvent;
}

void Controller::connect(std::string const& host, std::string const& service)
{
    server_.reset(new ServerConn(host, service, *this));
}

void Controller::send(std::string const& msg)
{
    if (!server_)
    {
        throw std::runtime_error("not connected");
    }
    server_->send(msg);
    timeLastSend_ = boost::chrono::steady_clock::now();
}

uint64_t Controller::lastSendTime() const
{
    auto const diff = timeLastSend_ - timeStart_;
    return duration_cast<milliseconds>(diff).count();
}

uint64_t Controller::timeNow() const
{
    auto const diff = steady_clock::now() - timeStart_;
    return duration_cast<milliseconds>(diff).count();
}

unsigned int Controller::startThread(boost::function<int()> function)
{
    unsigned int threadId = nextThreadId_;
    LOG(DEBUG) << "startThread " << threadId;

    boost::lock_guard<boost::mutex> lock(mutexThreads_); // protect threads_

    boost::thread* t = new boost::thread(boost::bind(&Controller::runThread, this, function, threadId));
    threads_[threadId] = ThreadInfo(t);

    ++nextThreadId_;
    nextThreadId_ = std::max(nextThreadId_, 1U); // make sure it doesn't wrap to zero

    return threadId;
}

void Controller::runThread(boost::function<int()> function, unsigned int id)
{
    LOG(DEBUG) << "runThread " << id;

    int const res = function();

    {
        boost::lock_guard<boost::mutex> lock(mutexThreads_);
        Threads::iterator it = threads_.find(id);
        LOG_IF(FATAL, it == threads_.end())<< "thread " << id << "not found";
        it->second.result_ = res;
    }

    ui_->addCallbackEvent(&threadDoneCallback, reinterpret_cast<void*>(static_cast<uintptr_t>(id)) );
}

void Controller::threadDoneCallback(void* data)
{
    unsigned int id = reinterpret_cast<uintptr_t>(data);

    boost::lock_guard<boost::mutex> lock(controller_->mutexThreads_); // protect threads_

    Threads::iterator it = controller_->threads_.find(id);
    LOG_IF(FATAL, it == controller_->threads_.end())<< "thread " << id << "not found";

    ThreadInfo& ti = it->second;

    ti.thread_->join();
    delete ti.thread_;

    controller_->client_->processDone(std::make_pair(id, ti.result_));

    controller_->threads_.erase(it);
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
