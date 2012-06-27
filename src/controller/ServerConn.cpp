#include "ServerConn.h"
#include "IServerEvent.h"
#include "log/Log.h"

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <cassert>

using boost::asio::ip::tcp;


ServerConn::ServerConn(std::string const & host, std::string const & service, IServerEvent & iServerEvent):
    client_(iServerEvent),
    socket_(ioService_),
    resolver_(ioService_)
{
    tcp::resolver::query query(host, service);

    resolver_.async_resolve(
            query,
            boost::bind(&ServerConn::resolveHandler, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::iterator));

    thread_.reset( new boost::thread( boost::bind(&boost::asio::io_service::run, &ioService_) ) );
}

ServerConn::~ServerConn()
{
    doClose();
    thread_->join();
    LOG(DEBUG) << "ServerConn destroyed";
}



//void ServerConn::connect(const std::string & host, const std::string & service)
//{
//    tcp::resolver::query query(host, service);
//
//    resolver_.async_resolve(
//            query,
//            boost::bind(&ServerConn::resolveHandler, this,
//                    boost::asio::placeholders::error,
//                    boost::asio::placeholders::iterator));
//
//    thread_.reset( new boost::thread( boost::bind(&boost::asio::io_service::run, &ioService_) ) );
//}

//void ServerConn::disconnect()
//{
//    doClose();
//}

void ServerConn::resolveHandler(const boost::system::error_code& error,
        boost::asio::ip::tcp::resolver::iterator iterator)
{
    if (!error)
    {
        boost::asio::async_connect(
                socket_,
                iterator,
                boost::bind(&ServerConn::connectHandler, this,
                        boost::asio::placeholders::error));
    }
    else
    {
        LOG(WARNING) << "resolve failed";
        client_.connected(false);
    }

}

void ServerConn::connectHandler(const boost::system::error_code& error)
{
    if (!error)
    {
        client_.connected(true);
//        connectedSignal_(true);
        boost::asio::async_read_until(
                socket_,
                recvBuf_,
                '\n',
                boost::bind(&ServerConn::readHandler, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred) );
    }
    else
    {
        LOG(WARNING) << "connect failed";
        doClose();
    }
}

void ServerConn::readHandler(const boost::system::error_code& error, std::size_t bytes)
{
    if (!error)
    {
        std::istream is(&recvBuf_);
        std::string line;
        std::getline(is, line);
        client_.message(line);
//        msgSignal_(line);

        boost::asio::async_read_until(
                socket_,
                recvBuf_,
                '\n',
                boost::bind(&ServerConn::readHandler, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        LOG(DEBUG) << "read failed";
        doClose();
    }
}

void ServerConn::send(std::string msg)
{
    assert(msg.size() > 0);
    if (msg.back() != '\n')
    {
        msg.append(1, '\n');
    }
    LOG(DEBUG) << "ServerConn::send " << msg;
    ioService_.post(boost::bind(&ServerConn::doSend, this, msg));
}

void ServerConn::doSend(const std::string & msg)
{
    bool write_in_progress = !sendQueue_.empty();
    sendQueue_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(
                socket_,
                boost::asio::buffer(sendQueue_.front().data(),
                        sendQueue_.front().length()),
                boost::bind(&ServerConn::writeHandler, this,
                        boost::asio::placeholders::error));
    }
}

void ServerConn::writeHandler(const boost::system::error_code& error)
{
    if (!error)
    {
        sendQueue_.pop_front();
        if (!sendQueue_.empty())
        {
            boost::asio::async_write(
                    socket_,
                    boost::asio::buffer(sendQueue_.front().data(),
                            sendQueue_.front().length()),
                    boost::bind(&ServerConn::writeHandler, this,
                            boost::asio::placeholders::error));
        }
    }
    else
    {
        doClose();
    }
}

//void ServerConn::setIServerEvent(IServerEvent & iServerEvent)
//{
//    client_ = &iServerEvent;
//}

void ServerConn::doClose()
{
    boost::lock_guard<boost::mutex> lock(mutex_);

    if (socket_.is_open())
    {
        socket_.close();
        LOG(DEBUG) << "closed";
        client_.connected(false);

    }
}

//// signal subscription
////
//ServerConn::Connection ServerConn::subscribeConnected(ConnectedSignal::slot_type subscriber)
//{
//    return connectedSignal_.connect(subscriber);
//}
//
//ServerConn::Connection ServerConn::subscribeMsg(MsgSignal::slot_type subscriber)
//{
//    return msgSignal_.connect(subscriber);
//}
