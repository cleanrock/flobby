#pragma once

#include <boost/asio.hpp>
#include <boost/signal.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <deque>
#include <memory>

// forwards
//
class IServerEvent;
namespace boost
{
    class thread;
}

class ServerConn
{
public:
    ServerConn(std::string const & host, std::string const & service, IServerEvent & iServerEvent);
    virtual ~ServerConn();

    void send(std::string msg);

//    // signals
//    //
//
//    typedef boost::signal<void (bool)> ConnectedSignal;
//    Connection subscribeConnected(ConnectedSignal::slot_type subscriber);
//
//    typedef boost::signal<void (const std::string msg)> MsgSignal;
//    Connection subscribeMsg(MsgSignal::slot_type subscriber);

private:
    IServerEvent & client_;
    boost::mutex mutex_;
    boost::asio::io_service ioService_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::streambuf recvBuf_;
    std::unique_ptr<boost::thread> thread_;

    typedef std::deque<std::string> SendQueue;
    SendQueue sendQueue_;

    void resolveHandler(
        const boost::system::error_code& error,
        boost::asio::ip::tcp::resolver::iterator iterator);
    void connectHandler(const boost::system::error_code& error);
    void readHandler(const boost::system::error_code& error, std::size_t bytes);

    void doSend(const std::string &     msg);
    void writeHandler(const boost::system::error_code& error);

    void doClose();

//    // signals
//    //
//    ConnectedSignal connectedSignal_;
//    MsgSignal msgSignal_;

};
