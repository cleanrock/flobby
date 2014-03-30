// This file is part of flobby (GPL v2 or later), see the LICENSE file

#pragma once

#include <boost/asio.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/array.hpp>
#include <mutex>
#include <deque>
#include <memory>

// forwards
class IServerEvent;
namespace std { class thread; }

class ServerConn
{
public:
    ServerConn(std::string const & host, std::string const & service, IServerEvent & iServerEvent);
    virtual ~ServerConn();

    void send(std::string msg);

private:
    IServerEvent & client_;
    std::mutex mutex_;
    boost::asio::io_service ioService_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::streambuf recvBuf_;
    std::unique_ptr<std::thread> thread_;

    typedef std::deque<std::string> SendQueue;
    SendQueue sendQueue_;

    void resolveHandler(
        const boost::system::error_code& error,
        boost::asio::ip::tcp::resolver::iterator iterator);
    void connectHandler(const boost::system::error_code& error);
    void readHandler(const boost::system::error_code& error, std::size_t bytes);

    void doSend(const std::string & msg);
    void writeHandler(const boost::system::error_code& error);

    void doClose();

};
