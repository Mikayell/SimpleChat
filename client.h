#include <iostream>
#include <boost/asio.hpp>
#include "net/connection.h"
#include "net/thread_safe_queue.h"

using namespace boost::asio;
using namespace boost::system;


class Client
{

public:
    Client()
    : _socket(_context)
    {}

    virtual ~Client()
    {
        Disconnect();
    }

public:

    bool Connect(const std::string& host, const int& port)
    {
        try
        {
            _connection = std::make_unique<Connection>();

            ip::tcp::resolver resolver(_context);
            auto endpoint = resolver.resolve(host, std::to_string(port));
            _connection -> ConnectToServer(endpoint);           // Connection method to be defined
            _thread = std::thread([this](){_context.run();});

        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        
    }
    bool Disconnect()
    {
        if(IsConnected())
            _connection -> Disconnect();  // Connection method to be defined
        
        _context.stop();    // we are done with context, and should stop it

        if(_thread.joinable())
            _thread.join();
        
        _connection.release(); // delete the unique pointer(Connection)
        
    }
    bool IsConnected();

private:

    TSQueue<std::string> _queue; // thread safe queue: to be defined

    io_context _context;
    std::thread _thread;
    ip::tcp::socket _socket;
    std::unique_ptr<Connection> _connection;
};
